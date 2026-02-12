---
layout: default
title: Node
---

# Node

## Overview

Central AST representation. Each `node` is a parsed C/C++ declaration with kind, type info, source location, tags, and tree structure. All queries and metadata extraction operate on nodes.

## Why Use This?

`node` is the primary data structure returned by `parser::parse()`. It provides a persistent, queryable tree that caches all relevant data from libclang cursors, enabling tree traversal, pattern matching, and filtering without raw libclang API exposure.

## Types

| Type | Description |
|------|-------------|
| `node` | AST node representing a parsed declaration |
| `node_ptr` | `std::shared_ptr<node>` — shared ownership pointer |
| `node_weak_ptr` | `std::weak_ptr<node>` — non-owning reference |
| `node::kind` | Enum of declaration kinds (40+ values) |
| `access_specifier` | Enum: `invalid`, `public_`, `protected_`, `private_` |
| `storage_class` | Enum: `none`, `extern_`, `static_`, `register_`, `auto_`, `thread_local_` |

### Node Kinds

**Type declarations:** `class_decl`, `struct_decl`, `union_decl`, `enum_decl`, `enum_constant_decl`, `typedef_decl`, `type_alias_decl`

**Members:** `field_decl`, `method_decl`, `constructor_decl`, `destructor_decl`, `conversion_decl`

**Functions:** `function_decl`, `function_template`, `parameter_decl`, `variable_decl`

**Templates:** `class_template`, `template_type_parameter`, `template_non_type_parameter`, `template_template_parameter`

**Other:** `namespace_decl`, `namespace_alias`, `using_directive`, `using_declaration`, `base_specifier`, `friend_decl`, `linkage_spec`, `static_assert_decl`, `translation_unit`

## Functions

### Identity

#### `get_kind`

Get the node's declaration kind.

**Signature:** `kind get_kind() const`

#### `get_kind_name`

Get a human-readable string for the node's kind.

**Signature:** `const char* get_kind_name() const`

#### `kind_to_string` (static)

Convert a kind enum to string.

**Signature:** `static const char* kind_to_string(kind k)`

#### `get_usr`

Get the Unified Symbol Resolution identifier. Deterministic across parses of the same code.

**Signature:** `const std::string& get_usr() const`

#### `get_name`

Get the simple name (e.g., `"MyClass"`).

**Signature:** `const std::string& get_name() const`

#### `get_qualified_name`

Get the fully qualified name (e.g., `"ns::MyClass"`).

**Signature:** `const std::string& get_qualified_name() const`

#### `get_display_name`

Get the display name (may include parameters for functions).

**Signature:** `const std::string& get_display_name() const`

#### `get_mangled_name`

Get the mangled linker symbol name.

**Signature:** `const std::string& get_mangled_name() const`

### Type Information

#### `get_type`

Get type information for typed declarations (fields, variables).

**Signature:** `const type_info& get_type() const`

#### `get_return_type`

Get the return type for functions/methods.

**Signature:** `const type_info& get_return_type() const`

### Location

#### `get_location`

Get the source location.

**Signature:** `const source_location& get_location() const`

#### `get_extent`

Get the source range spanning the entire declaration.

**Signature:** `const source_range& get_extent() const`

### Access and Storage

#### `get_access`

Get the access specifier (public/protected/private).

**Signature:** `access_specifier get_access() const`

#### `get_storage_class`

Get the storage class specifier.

**Signature:** `storage_class get_storage_class() const`

### Declaration Properties

#### `is_definition`

Whether this is a definition (not just a forward declaration).

**Signature:** `bool is_definition() const`

#### `is_virtual` / `is_pure_virtual` / `is_override` / `is_final`

Virtual method properties.

**Signatures:**
- `bool is_virtual() const`
- `bool is_pure_virtual() const`
- `bool is_override() const`
- `bool is_final() const`

#### `is_static` / `is_const_method` / `is_inline` / `is_explicit`

Method qualifiers.

**Signatures:**
- `bool is_static() const`
- `bool is_const_method() const`
- `bool is_inline() const`
- `bool is_explicit() const`

#### `is_constexpr` / `is_noexcept` / `is_deleted` / `is_defaulted`

Special function properties.

**Signatures:**
- `bool is_constexpr() const`
- `bool is_noexcept() const`
- `bool is_deleted() const`
- `bool is_defaulted() const`

#### `is_anonymous` / `is_scoped_enum`

Type properties.

**Signatures:**
- `bool is_anonymous() const`
- `bool is_scoped_enum() const`

#### `is_template` / `is_template_specialization` / `is_variadic`

Template properties.

**Signatures:**
- `bool is_template() const`
- `bool is_template_specialization() const`
- `bool is_variadic() const`

#### `is_bitfield` / `get_bitfield_width`

Bitfield properties.

**Signatures:**
- `bool is_bitfield() const`
- `int get_bitfield_width() const`

### Default Values and Enum Info

#### `has_default_value` / `get_default_value`

**Signatures:**
- `bool has_default_value() const`
- `const std::string& get_default_value() const`

#### `get_underlying_type`

Get the underlying type for enums.

**Signature:** `const std::string& get_underlying_type() const`

#### `get_enum_value`

Get the integer value of an enum constant.

**Signature:** `std::int64_t get_enum_value() const`

#### `is_virtual_base`

Whether a base specifier is virtual.

**Signature:** `bool is_virtual_base() const`

### Documentation

#### `get_comment` / `get_brief_comment`

Get documentation comments attached to the declaration.

**Signatures:**
- `const std::string& get_comment() const` — full comment text
- `const std::string& get_brief_comment() const` — first paragraph only

### Tags

#### `get_tags`

Get all tags on this node.

**Signature:** `const std::vector<tag>& get_tags() const`

#### `has_tag`

Check if a specific tag exists.

**Signature:** `bool has_tag(const std::string& name) const`

#### `has_tags`

Check if any of the given tags exist.

**Signature:** `bool has_tags(const std::vector<std::string>& names) const`

#### `find_tag`

Find the first tag matching a name.

**Signature:** `std::optional<tag> find_tag(const std::string& name) const`

**Returns:** the tag if found, `std::nullopt` otherwise

#### `find_tags`

Find all tags matching any of the given names.

**Signature:** `std::vector<tag> find_tags(const std::vector<std::string>& names) const`

#### `get_parent_tags`

Get tags from all ancestor nodes (walking up the tree).

**Signature:** `std::vector<tag> get_parent_tags() const`

#### `get_all_tags`

Get own tags combined with parent tags.

**Signature:** `std::vector<tag> get_all_tags() const`

### Tree Navigation

#### `get_parent`

Get the parent node.

**Signature:** `node_ptr get_parent() const`

**Returns:** parent node, or `nullptr` for root

#### `get_children`

Get all child nodes.

**Signature:** `const std::vector<node_ptr>& get_children() const`

#### `find_child`

Find the first child matching a predicate.

**Signature:** `template <typename Predicate> node_ptr find_child(Predicate pred) const`

**Returns:** first matching child, or `nullptr`

#### `find_children`

Find all children matching a predicate.

**Signature:** `template <typename Predicate> std::vector<node_ptr> find_children(Predicate pred) const`

#### `find_descendants`

Recursively find all descendants matching a predicate (depth-first).

**Signature:** `template <typename Predicate> std::vector<node_ptr> find_descendants(Predicate pred) const`

**Notes:** For large trees (>10k nodes), consider caching results or using `filter`.

### Convenience Queries

#### `get_children_by_kind`

**Signature:** `std::vector<node_ptr> get_children_by_kind(kind k) const`

#### `find_child_by_name`

**Signature:** `node_ptr find_child_by_name(const std::string& name) const`

#### Tag-Based Child Queries

**Signatures:**
- `std::vector<node_ptr> get_children_by_tag(const std::string& tag_name) const`
- `std::vector<node_ptr> get_children_by_tags(const std::vector<std::string>& tag_names) const`
- `std::vector<node_ptr> get_children_without_tag(const std::string& tag_name) const`
- `std::vector<node_ptr> get_children_without_tags(const std::vector<std::string>& tag_names) const`
- `node_ptr find_child_with_tag(const std::string& tag_name) const`
- `node_ptr find_child_with_tags(const std::vector<std::string>& tag_names) const`
- `node_ptr find_child_without_tag(const std::string& tag_name) const`
- `node_ptr find_child_without_tags(const std::vector<std::string>& tag_names) const`

#### Type Helpers

**Signatures:**
- `bool is_type_decl() const` — class/struct/union/enum/typedef?
- `bool is_record_decl() const` — class/struct/union?
- `bool is_callable() const` — function/method/constructor?

#### Member Accessors

**Signatures:**
- `std::vector<node_ptr> get_bases() const` — base classes
- `std::vector<node_ptr> get_methods() const` — methods
- `std::vector<node_ptr> get_fields() const` — fields
- `std::vector<node_ptr> get_parameters() const` — function/method parameters
- `std::vector<node_ptr> get_enum_constants() const` — enum values

### Free Functions

#### `access_specifier_to_string`

**Signature:** `const char* access_specifier_to_string(access_specifier a)`

#### `storage_class_to_string`

**Signature:** `const char* storage_class_to_string(storage_class sc)`

**Example:**

```cpp
auto structs = ast->find_descendants([](auto& n) {
  return n->is_record_decl() && n->has_tag("reflect");
});

for (auto& s : structs) {
  for (auto& field : s->get_fields()) {
    auto type = field->get_type().get_spelling();
    auto name = field->get_name();
  }
}
```
