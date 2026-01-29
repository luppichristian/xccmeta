/*
MIT License

Copyright (c) 2026 Christian Luppi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "xccmeta_base.hpp"
#include "xccmeta_source.hpp"
#include "xccmeta_tags.hpp"
#include "xccmeta_type_info.hpp"

namespace xccmeta {

  // Forward declarations
  class node;
  using node_ptr = std::shared_ptr<node>;
  using node_weak_ptr = std::weak_ptr<node>;

  // Access specifiers for class members
  enum class access_specifier {
    invalid,
    public_,
    protected_,
    private_
  };

  // Storage class specifiers
  enum class storage_class {
    none,
    extern_,
    static_,
    register_,
    auto_,
    thread_local_
  };

  // AST Node - represents a parsed declaration/definition
  class XCCMETA_API node : public std::enable_shared_from_this<node> {
    friend class parser;
    friend class parser_impl;
    friend class type_info;

    // Private key for passkey idiom - allows make_shared while keeping constructors effectively private
    struct private_key {
      explicit private_key() = default;
    };
    friend struct private_key;

   public:
    // -------------------------------------------------------------------------
    // Node kind enumeration covering C/C++ declarations
    // -------------------------------------------------------------------------
    enum class kind {
      unknown,

      // Root of the translation unit
      translation_unit,

      // Namespaces
      namespace_decl,
      namespace_alias,
      using_directive,
      using_declaration,

      // Type declarations
      class_decl,
      struct_decl,
      union_decl,
      enum_decl,
      enum_constant_decl,
      typedef_decl,
      type_alias_decl,  // C++11 using = ...

      // Class/struct members
      field_decl,
      method_decl,
      constructor_decl,
      destructor_decl,
      conversion_decl,

      // Functions
      function_decl,
      function_template,
      parameter_decl,

      // Variables
      variable_decl,

      // Templates
      class_template,
      template_type_parameter,
      template_non_type_parameter,
      template_template_parameter,

      // Friend declarations
      friend_decl,

      // Inheritance
      base_specifier,

      // Linkage specification
      linkage_spec,

      // Static assert
      static_assert_decl,
    };

    // Public constructors using passkey idiom (can only be called with private_key)
    explicit node(private_key, kind k = kind::unknown);

    // Construction / Destruction
    virtual ~node() = default;
    node(const node&) = delete;
    node& operator=(const node&) = delete;
    node(node&&) noexcept = default;
    node& operator=(node&&) noexcept = default;

    // Node kind and identity
    kind get_kind() const { return kind_; }

    // Human-readable kind name
    const char* get_kind_name() const;
    static const char* kind_to_string(kind k);

    // Unique identifier (e.g., USR - Unified Symbol Resolution)
    const std::string& get_usr() const { return usr_; }

    // Simple name (e.g., "MyClass")
    const std::string& get_name() const { return name_; }

    // Fully qualified name (e.g., "my_namespace::MyClass")
    const std::string& get_qualified_name() const { return qualified_name_; }

    // Display name (may include parameters for functions)
    const std::string& get_display_name() const { return display_name_; }

    // Mangled name (for linker symbols)
    const std::string& get_mangled_name() const { return mangled_name_; }

    // Source location
    const source_location& get_location() const { return location_; }
    const source_range& get_extent() const { return extent_; }

    // Type information (for typed declarations)
    const type_info& get_type() const { return type_; }
    const type_info& get_return_type() const { return return_type_; }  // Return type (for functions/methods)

    // Access and storage
    access_specifier get_access() const { return access_; }
    storage_class get_storage_class() const { return storage_class_; }

    // Declaration properties
    bool is_definition() const { return is_definition_; }  // Definition vs declaration
    bool is_virtual() const { return is_virtual_; }
    bool is_pure_virtual() const { return is_pure_virtual_; }
    bool is_override() const { return is_override_; }
    bool is_final() const { return is_final_; }
    bool is_static() const { return is_static_; }
    bool is_const_method() const { return is_const_method_; }
    bool is_inline() const { return is_inline_; }
    bool is_explicit() const { return is_explicit_; }
    bool is_constexpr() const { return is_constexpr_; }
    bool is_noexcept() const { return is_noexcept_; }
    bool is_deleted() const { return is_deleted_; }
    bool is_defaulted() const { return is_defaulted_; }
    bool is_anonymous() const { return is_anonymous_; }
    bool is_scoped_enum() const { return is_scoped_enum_; }
    bool is_template() const { return is_template_; }
    bool is_template_specialization() const { return is_template_spec_; }
    bool is_variadic() const { return is_variadic_; }
    bool is_bitfield() const { return is_bitfield_; }  // Definition vs declaration
    int get_bitfield_width() const { return bitfield_width_; }

    // Default values / initializers
    bool has_default_value() const { return has_default_value_; }
    const std::string& get_default_value() const { return default_value_; }

    // Enum underlying type
    const std::string& get_underlying_type() const { return underlying_type_; }

    // Enum constant value
    std::int64_t get_enum_value() const { return enum_value_; }

    // Base class specifier info
    bool is_virtual_base() const { return is_virtual_base_; }

    // Documentation comment
    const std::string& get_comment() const { return comment_; }
    const std::string& get_brief_comment() const { return brief_comment_; }

    // xccmeta tags (metadata annotations)
    const std::vector<tag>& get_tags() const { return tags_; }
    bool has_tag(const std::string& name) const;
    bool has_tags(const std::vector<std::string>& names) const;  // Returns true if any of the tags are present
    std::optional<tag> find_tag(const std::string& name) const;
    std::vector<tag> find_tags(const std::vector<std::string>& names) const;  // Find all tags matching any of the given names

    // Tree structure
    node_ptr get_parent() const { return parent_.lock(); }
    const std::vector<node_ptr>& get_children() const { return children_; }

    // Find first child matching predicate
    template <typename Predicate>
    node_ptr find_child(Predicate pred) const {
      for (const auto& child : children_) {
        if (pred(child)) return child;
      }
      return nullptr;
    }

    // Find all children matching predicate
    template <typename Predicate>
    std::vector<node_ptr> find_children(Predicate pred) const {
      std::vector<node_ptr> result;
      for (const auto& child : children_) {
        if (pred(child)) result.push_back(child);
      }
      return result;
    }

    // Find descendants (recursive) matching predicate
    template <typename Predicate>
    std::vector<node_ptr> find_descendants(Predicate pred) const {
      std::vector<node_ptr> result;
      find_descendants_impl(pred, result);
      return result;
    }

    // Get children by kind
    std::vector<node_ptr> get_children_by_kind(kind k) const;

    // Find child by name
    node_ptr find_child_by_name(const std::string& name) const;

    // Tag-based child queries
    std::vector<node_ptr> get_children_by_tag(const std::string& tag_name) const;                      // Get all children that have a specific tag
    std::vector<node_ptr> get_children_by_tags(const std::vector<std::string>& tag_names) const;       // Get all children that have any of the specified tags
    std::vector<node_ptr> get_children_without_tag(const std::string& tag_name) const;                 // Get all children that don't have a specific tag
    std::vector<node_ptr> get_children_without_tags(const std::vector<std::string>& tag_names) const;  // Get all children that don't have any of the specified tags
    node_ptr find_child_with_tag(const std::string& tag_name) const;                                   // Find the first child that has a specific tag
    node_ptr find_child_with_tags(const std::vector<std::string>& tag_names) const;                    // Find the first child that has any of the specified tags
    node_ptr find_child_without_tag(const std::string& tag_name) const;                                // Find the first child that doesn't have a specific tag
    node_ptr find_child_without_tags(const std::vector<std::string>& tag_names) const;                 // Find the first child that doesn't have any of the specified tags

    // Parent tag queries
    std::vector<tag> get_parent_tags() const;  // Get all tags from parent nodes (walking up the tree)
    std::vector<tag> get_all_tags() const;     // Get all tags (own tags + parent tags combined)

    // Convenience queries
    bool is_type_decl() const;                         // Is this a type declaration (class/struct/union/enum/typedef)?
    bool is_record_decl() const;                       // Is this a record type (class/struct/union)?
    bool is_callable() const;                          // Is this a callable (function/method/constructor)?
    std::vector<node_ptr> get_bases() const;           // Get all base classes (for class/struct)
    std::vector<node_ptr> get_methods() const;         // Get all methods (for class/struct)
    std::vector<node_ptr> get_fields() const;          // Get all fields (for class/struct)
    std::vector<node_ptr> get_parameters() const;      // Get all parameters (for functions/methods)
    std::vector<node_ptr> get_enum_constants() const;  // Get all enum constants (for enums)

   protected:
    // Protected factory method (only accessible by parser)
    static node_ptr create(kind k = kind::unknown);

    // Setters (only accessible by parser)
    void set_kind(kind k) { kind_ = k; }
    void set_usr(const std::string& usr) { usr_ = usr; }
    void set_name(const std::string& name) { name_ = name; }
    void set_qualified_name(const std::string& name) { qualified_name_ = name; }
    void set_display_name(const std::string& name) { display_name_ = name; }
    void set_mangled_name(const std::string& name) { mangled_name_ = name; }
    void set_location(const source_location& loc) { location_ = loc; }
    void set_extent(const source_range& range) { extent_ = range; }

    type_info& get_type_mutable() { return type_; }
    void set_type(const type_info& t) { type_ = t; }
    type_info& get_return_type_mutable() { return return_type_; }
    void set_return_type(const type_info& t) { return_type_ = t; }

    void set_access(access_specifier a) { access_ = a; }
    void set_storage_class(storage_class sc) { storage_class_ = sc; }

    void set_definition(bool v) { is_definition_ = v; }
    void set_virtual(bool v) { is_virtual_ = v; }
    void set_pure_virtual(bool v) { is_pure_virtual_ = v; }
    void set_override(bool v) { is_override_ = v; }
    void set_final(bool v) { is_final_ = v; }
    void set_static(bool v) { is_static_ = v; }
    void set_const_method(bool v) { is_const_method_ = v; }
    void set_inline(bool v) { is_inline_ = v; }
    void set_explicit(bool v) { is_explicit_ = v; }
    void set_constexpr(bool v) { is_constexpr_ = v; }
    void set_noexcept(bool v) { is_noexcept_ = v; }
    void set_deleted(bool v) { is_deleted_ = v; }
    void set_defaulted(bool v) { is_defaulted_ = v; }
    void set_anonymous(bool v) { is_anonymous_ = v; }
    void set_scoped_enum(bool v) { is_scoped_enum_ = v; }
    void set_template(bool v) { is_template_ = v; }
    void set_template_specialization(bool v) { is_template_spec_ = v; }
    void set_variadic(bool v) { is_variadic_ = v; }
    void set_bitfield(bool v) { is_bitfield_ = v; }
    void set_bitfield_width(int w) { bitfield_width_ = w; }
    void set_has_default_value(bool v) { has_default_value_ = v; }
    void set_default_value(const std::string& v) { default_value_ = v; }
    void set_underlying_type(const std::string& t) { underlying_type_ = t; }
    void set_enum_value(std::int64_t v) { enum_value_ = v; }
    void set_virtual_base(bool v) { is_virtual_base_ = v; }
    void set_comment(const std::string& c) { comment_ = c; }
    void set_brief_comment(const std::string& c) { brief_comment_ = c; }

    std::vector<tag>& get_tags_mutable() { return tags_; }
    void add_tag(const tag& t) { tags_.push_back(t); }
    void add_tag(tag&& t) { tags_.push_back(std::move(t)); }

    void set_parent(node_ptr p) { parent_ = p; }
    std::vector<node_ptr>& get_children_mutable() { return children_; }
    void add_child(node_ptr child);
    void remove_child(const node_ptr& child);

   private:
    template <typename Predicate>
    void find_descendants_impl(Predicate pred, std::vector<node_ptr>& result) const {
      for (const auto& child : children_) {
        if (pred(child)) result.push_back(child);
        child->find_descendants_impl(pred, result);
      }
    }

    // Node identity
    kind kind_ = kind::unknown;
    std::string usr_;

    // Names
    std::string name_;
    std::string qualified_name_;
    std::string display_name_;
    std::string mangled_name_;

    // Location
    source_location location_;
    source_range extent_;

    // Type info
    type_info type_;
    type_info return_type_;

    // Access and storage
    access_specifier access_ = access_specifier::invalid;
    storage_class storage_class_ = storage_class::none;

    // Boolean properties
    bool is_definition_ = false;
    bool is_virtual_ = false;
    bool is_pure_virtual_ = false;
    bool is_override_ = false;
    bool is_final_ = false;
    bool is_static_ = false;
    bool is_const_method_ = false;
    bool is_inline_ = false;
    bool is_explicit_ = false;
    bool is_constexpr_ = false;
    bool is_noexcept_ = false;
    bool is_deleted_ = false;
    bool is_defaulted_ = false;
    bool is_anonymous_ = false;
    bool is_scoped_enum_ = false;
    bool is_template_ = false;
    bool is_template_spec_ = false;
    bool is_variadic_ = false;
    bool is_bitfield_ = false;
    bool is_virtual_base_ = false;
    bool has_default_value_ = false;

    // Numeric values
    int bitfield_width_ = 0;
    std::int64_t enum_value_ = 0;

    // String values
    std::string default_value_;
    std::string underlying_type_;
    std::string comment_;
    std::string brief_comment_;

    // Tags
    std::vector<tag> tags_;

    // Tree structure
    node_weak_ptr parent_;
    std::vector<node_ptr> children_;
  };

  // Utility: Convert enum to string and vice versa
  XCCMETA_API const char* access_specifier_to_string(access_specifier a);
  XCCMETA_API const char* storage_class_to_string(storage_class sc);

}  // namespace xccmeta