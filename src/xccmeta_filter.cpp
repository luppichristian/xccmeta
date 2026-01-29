#include <xccmeta/xccmeta_filter.hpp>

#include <algorithm>

namespace xccmeta {

  filter::filter(const config& cfg): config_(cfg) {
  }

  filter& filter::clean() {
    types_.erase(
        std::remove_if(types_.begin(), types_.end(), [this](const node_ptr& type) {
          return !matches_config(type);
        }),
        types_.end());
    return *this;
  }

  bool filter::contains(const node_ptr& type) const {
    if (!type) return false;
    return find_by_usr(type->get_usr()) != types_.end();
  }

  bool filter::add(const node_ptr& type) {
    if (!type) return false;
    if (!is_valid_type(type)) return false;
    if (contains(type)) return false;

    types_.push_back(type);
    return true;
  }

  bool filter::remove(const node_ptr& type) {
    if (!type) return false;

    auto it = find_by_usr(type->get_usr());
    if (it == types_.end()) return false;

    types_.erase(it);
    return true;
  }

  filter& filter::clear() {
    types_.clear();
    return *this;
  }

  std::size_t filter::size() const {
    return types_.size();
  }

  bool filter::empty() const {
    return types_.empty();
  }

  const std::vector<node_ptr>& filter::get_types() const {
    return types_;
  }

  const filter::config& filter::get_config() const {
    return config_;
  }

  bool filter::is_valid_type(const node_ptr& type) const {
    if (!type) return false;

    // Check if it's a type declaration
    if (!type->is_type_decl()) return false;

    // Check config-based filtering
    return matches_config(type);
  }

  std::vector<node_ptr>::iterator filter::find_by_usr(const std::string& usr) {
    return std::find_if(types_.begin(), types_.end(), [&usr](const node_ptr& n) {
      return n && n->get_usr() == usr;
    });
  }

  std::vector<node_ptr>::const_iterator filter::find_by_usr(const std::string& usr) const {
    return std::find_if(types_.cbegin(), types_.cend(), [&usr](const node_ptr& n) {
      return n && n->get_usr() == usr;
    });
  }

  bool filter::matches_config(const node_ptr& type) const {
    if (!type) return false;

    const auto kind = type->get_kind();

    // Check allowed_kinds filter (if not empty, only allow types in the list)
    if (!config_.allowed_kinds.empty()) {
      bool kind_allowed = std::find(config_.allowed_kinds.begin(),
                                    config_.allowed_kinds.end(),
                                    kind) != config_.allowed_kinds.end();
      if (!kind_allowed) return false;
    }

    // Check tag-based filtering
    bool has_tags = !type->get_tags().empty();

    if (has_tags) {
      // Check avoid_tag_names - if type has any of these tags, exclude it
      for (const auto& avoid_tag : config_.avoid_tag_names) {
        if (type->has_tag(avoid_tag)) {
          return false;
        }
      }

      // Check grab_tag_names - if not empty, type must have at least one of these tags
      if (!config_.grab_tag_names.empty()) {
        bool has_grab_tag = false;
        for (const auto& grab_tag : config_.grab_tag_names) {
          if (type->has_tag(grab_tag)) {
            has_grab_tag = true;
            break;
          }
        }
        if (!has_grab_tag) return false;
      }
    } else {
      // Type has no tags - check if we should include untagged types
      // If grab_tag_names is not empty and type has no tags, exclude it
      // (unless child/parent inclusion allows it, but that's handled elsewhere)
      if (!config_.grab_tag_names.empty()) {
        return false;
      }
    }

    return true;
  }

}  // namespace xccmeta