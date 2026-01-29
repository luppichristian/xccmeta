#include "xccmeta/xccmeta_node.hpp"

#include <algorithm>

namespace xccmeta {

  node::node(node::private_key, kind k): kind_(k) {
  }

  node_ptr node::create(kind k) {
    return std::make_shared<node>(private_key {}, k);
  }

  const char* node::get_kind_name() const {
    return kind_to_string(kind_);
  }

  const char* node::kind_to_string(kind k) {
    switch (k) {
      case kind::unknown:                     return "unknown";
      case kind::translation_unit:            return "translation_unit";
      case kind::namespace_decl:              return "namespace_decl";
      case kind::namespace_alias:             return "namespace_alias";
      case kind::using_directive:             return "using_directive";
      case kind::using_declaration:           return "using_declaration";
      case kind::class_decl:                  return "class_decl";
      case kind::struct_decl:                 return "struct_decl";
      case kind::union_decl:                  return "union_decl";
      case kind::enum_decl:                   return "enum_decl";
      case kind::enum_constant_decl:          return "enum_constant_decl";
      case kind::typedef_decl:                return "typedef_decl";
      case kind::type_alias_decl:             return "type_alias_decl";
      case kind::field_decl:                  return "field_decl";
      case kind::method_decl:                 return "method_decl";
      case kind::constructor_decl:            return "constructor_decl";
      case kind::destructor_decl:             return "destructor_decl";
      case kind::conversion_decl:             return "conversion_decl";
      case kind::function_decl:               return "function_decl";
      case kind::function_template:           return "function_template";
      case kind::parameter_decl:              return "parameter_decl";
      case kind::variable_decl:               return "variable_decl";
      case kind::class_template:              return "class_template";
      case kind::template_type_parameter:     return "template_type_parameter";
      case kind::template_non_type_parameter: return "template_non_type_parameter";
      case kind::template_template_parameter: return "template_template_parameter";
      case kind::friend_decl:                 return "friend_decl";
      case kind::base_specifier:              return "base_specifier";
      case kind::linkage_spec:                return "linkage_spec";
      case kind::static_assert_decl:          return "static_assert_decl";
      default:                                return "unknown";
    }
  }

  bool node::has_tag(const std::string& name) const {
    return std::any_of(tags_.begin(), tags_.end(), [&name](const tag& t) { return t.get_name() == name; });
  }

  bool node::has_tags(const std::vector<std::string>& names) const {
    for (const auto& name : names) {
      if (has_tag(name)) return true;
    }
    return false;
  }

  std::optional<tag> node::find_tag(const std::string& name) const {
    auto it = std::find_if(tags_.begin(), tags_.end(), [&name](const tag& t) { return t.get_name() == name; });
    if (it != tags_.end()) return *it;
    return std::nullopt;
  }

  std::vector<tag> node::find_tags(const std::vector<std::string>& names) const {
    std::vector<tag> result;
    for (const auto& t : tags_) {
      for (const auto& name : names) {
        if (t.get_name() == name) {
          result.push_back(t);
          break;
        }
      }
    }
    return result;
  }

  void node::add_child(node_ptr child) {
    if (child) {
      child->parent_ = shared_from_this();
      children_.push_back(std::move(child));
    }
  }

  void node::remove_child(const node_ptr& child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
      (*it)->parent_.reset();
      children_.erase(it);
    }
  }

  std::vector<node_ptr> node::get_children_by_kind(kind k) const {
    std::vector<node_ptr> result;
    for (const auto& child : children_) {
      if (child->get_kind() == k) result.push_back(child);
    }
    return result;
  }

  node_ptr node::find_child_by_name(const std::string& name) const {
    for (const auto& child : children_) {
      if (child->get_name() == name) return child;
    }
    return nullptr;
  }

  bool node::is_type_decl() const {
    switch (kind_) {
      case kind::class_decl:
      case kind::struct_decl:
      case kind::union_decl:
      case kind::enum_decl:
      case kind::typedef_decl:
      case kind::type_alias_decl:
        return true;
      default:
        return false;
    }
  }

  bool node::is_record_decl() const {
    switch (kind_) {
      case kind::class_decl:
      case kind::struct_decl:
      case kind::union_decl:
        return true;
      default:
        return false;
    }
  }

  bool node::is_callable() const {
    switch (kind_) {
      case kind::function_decl:
      case kind::function_template:
      case kind::method_decl:
      case kind::constructor_decl:
      case kind::destructor_decl:
      case kind::conversion_decl:
        return true;
      default:
        return false;
    }
  }

  std::vector<node_ptr> node::get_bases() const {
    return get_children_by_kind(kind::base_specifier);
  }

  std::vector<node_ptr> node::get_methods() const {
    std::vector<node_ptr> result;
    for (const auto& child : children_) {
      if (child->get_kind() == kind::method_decl ||
          child->get_kind() == kind::constructor_decl ||
          child->get_kind() == kind::destructor_decl ||
          child->get_kind() == kind::conversion_decl) {
        result.push_back(child);
      }
    }
    return result;
  }

  std::vector<node_ptr> node::get_fields() const {
    return get_children_by_kind(kind::field_decl);
  }

  std::vector<node_ptr> node::get_parameters() const {
    return get_children_by_kind(kind::parameter_decl);
  }

  std::vector<node_ptr> node::get_enum_constants() const {
    return get_children_by_kind(kind::enum_constant_decl);
  }

  // =============================================================================
  // Tag-based child queries
  // =============================================================================

  std::vector<node_ptr> node::get_children_by_tag(const std::string& tag_name) const {
    std::vector<node_ptr> result;
    for (const auto& child : children_) {
      if (child->has_tag(tag_name)) {
        result.push_back(child);
      }
    }
    return result;
  }

  std::vector<node_ptr> node::get_children_by_tags(const std::vector<std::string>& tag_names) const {
    std::vector<node_ptr> result;
    for (const auto& child : children_) {
      for (const auto& tag_name : tag_names) {
        if (child->has_tag(tag_name)) {
          result.push_back(child);
          break;  // Don't add the same child multiple times
        }
      }
    }
    return result;
  }

  std::vector<node_ptr> node::get_children_without_tag(const std::string& tag_name) const {
    std::vector<node_ptr> result;
    for (const auto& child : children_) {
      if (!child->has_tag(tag_name)) {
        result.push_back(child);
      }
    }
    return result;
  }

  std::vector<node_ptr> node::get_children_without_tags(const std::vector<std::string>& tag_names) const {
    std::vector<node_ptr> result;
    for (const auto& child : children_) {
      if (!child->has_tags(tag_names)) {
        result.push_back(child);
      }
    }
    return result;
  }

  node_ptr node::find_child_with_tag(const std::string& tag_name) const {
    for (const auto& child : children_) {
      if (child->has_tag(tag_name)) {
        return child;
      }
    }
    return nullptr;
  }

  node_ptr node::find_child_with_tags(const std::vector<std::string>& tag_names) const {
    for (const auto& child : children_) {
      if (child->has_tags(tag_names)) {
        return child;
      }
    }
    return nullptr;
  }

  node_ptr node::find_child_without_tag(const std::string& tag_name) const {
    for (const auto& child : children_) {
      if (!child->has_tag(tag_name)) {
        return child;
      }
    }
    return nullptr;
  }

  node_ptr node::find_child_without_tags(const std::vector<std::string>& tag_names) const {
    for (const auto& child : children_) {
      if (!child->has_tags(tag_names)) {
        return child;
      }
    }
    return nullptr;
  }

  // =============================================================================
  // Parent tag queries
  // =============================================================================

  std::vector<tag> node::get_parent_tags() const {
    std::vector<tag> result;
    node_ptr parent = parent_.lock();
    while (parent) {
      const auto& parent_tags = parent->get_tags();
      result.insert(result.end(), parent_tags.begin(), parent_tags.end());
      parent = parent->get_parent();
    }
    return result;
  }

  std::vector<tag> node::get_all_tags() const {
    std::vector<tag> result = tags_;  // Start with own tags
    std::vector<tag> parent_tags = get_parent_tags();
    result.insert(result.end(), parent_tags.begin(), parent_tags.end());
    return result;
  }

  // =============================================================================
  // Utility functions
  // =============================================================================

  const char* access_specifier_to_string(access_specifier a) {
    switch (a) {
      case access_specifier::invalid:    return "invalid";
      case access_specifier::public_:    return "public";
      case access_specifier::protected_: return "protected";
      case access_specifier::private_:   return "private";
      default:                           return "invalid";
    }
  }

  const char* storage_class_to_string(storage_class sc) {
    switch (sc) {
      case storage_class::none:          return "none";
      case storage_class::extern_:       return "extern";
      case storage_class::static_:       return "static";
      case storage_class::register_:     return "register";
      case storage_class::auto_:         return "auto";
      case storage_class::thread_local_: return "thread_local";
      default:                           return "none";
    }
  }

}  // namespace xccmeta
