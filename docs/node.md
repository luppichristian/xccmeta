# Node

The `node` class represents a single declaration in the AST (Abstract Syntax Tree). It provides access to all metadata about C/C++ declarations including names, types, attributes, and tree structure.

## Header

```cpp
#include <xccmeta/xccmeta_node.hpp>
```

## Overview

Each `node` represents a C/C++ declaration such as:
- Classes, structs, unions, enums
- Functions and methods
- Variables and fields
- Namespaces and type aliases
- And more...

Nodes form a tree structure where the root is a `translation_unit` and children are the declarations within.

## Type Aliases

```cpp
using node_ptr = std::shared_ptr<node>;
using node_weak_ptr = std::weak_ptr<node>;
```

## Node Kinds

The `node::kind` enum defines all supported declaration types:

```cpp
enum class kind {
    unknown,
    
    // Root
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
    
    // Other
    friend_decl,
    base_specifier,
    linkage_spec,
    static_assert_decl,
};
```

## Access Specifiers

```cpp
enum class access_specifier {
    invalid,
    public_,
    protected_,
    private_
};
```

## Storage Classes

```cpp
enum class storage_class {
    none,
    extern_,
    static_,
    register_,
    auto_,
    thread_local_
};
```

## Identity and Names

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_kind()` | `kind` | The node kind |
| `get_kind_name()` | `const char*` | Human-readable kind name |
| `get_usr()` | `const std::string&` | Unified Symbol Resolution (unique ID) |
| `get_name()` | `const std::string&` | Simple name (e.g., "MyClass") |
| `get_qualified_name()` | `const std::string&` | Fully qualified name (e.g., "ns::MyClass") |
| `get_display_name()` | `const std::string&` | Display name (may include parameters) |
| `get_mangled_name()` | `const std::string&` | Mangled name for linker |

### Example

```cpp
auto node = /* ... */;
std::cout << "Kind: " << node->get_kind_name() << "\n";
std::cout << "Name: " << node->get_name() << "\n";
std::cout << "Qualified: " << node->get_qualified_name() << "\n";
```

## Source Location

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_location()` | `const source_location&` | Start location in source |
| `get_extent()` | `const source_range&` | Full extent (start to end) |

### Example

```cpp
auto loc = node->get_location();
std::cout << "Defined at: " << loc.file << ":" << loc.line << "\n";
```

## Type Information

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_type()` | `const type_info&` | Type of the declaration |
| `get_return_type()` | `const type_info&` | Return type (for functions) |

### Example

```cpp
if (node->get_kind() == xccmeta::node::kind::field_decl) {
    auto& type = node->get_type();
    std::cout << "Field type: " << type.get_spelling() << "\n";
    std::cout << "Is pointer: " << type.is_pointer() << "\n";
}
```

## Access and Storage

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_access()` | `access_specifier` | Access level (public/protected/private) |
| `get_storage_class()` | `storage_class` | Storage class specifier |

## Declaration Properties

Boolean properties about the declaration:

| Method | Description |
|--------|-------------|
| `is_definition()` | Is this a definition (not just declaration)? |
| `is_virtual()` | Is virtual? |
| `is_pure_virtual()` | Is pure virtual (= 0)? |
| `is_override()` | Has override specifier? |
| `is_final()` | Has final specifier? |
| `is_static()` | Is static? |
| `is_const_method()` | Is const method? |
| `is_inline()` | Is inline? |
| `is_explicit()` | Is explicit (constructor/conversion)? |
| `is_constexpr()` | Is constexpr? |
| `is_noexcept()` | Is noexcept? |
| `is_deleted()` | Is deleted (= delete)? |
| `is_defaulted()` | Is defaulted (= default)? |
| `is_anonymous()` | Is anonymous type? |
| `is_scoped_enum()` | Is scoped enum (enum class)? |
| `is_template()` | Is a template? |
| `is_template_specialization()` | Is template specialization? |
| `is_variadic()` | Is variadic function? |
| `is_bitfield()` | Is bitfield? |
| `get_bitfield_width()` | Bitfield width in bits |

## Default Values

| Method | Return Type | Description |
|--------|-------------|-------------|
| `has_default_value()` | `bool` | Has a default value/initializer? |
| `get_default_value()` | `const std::string&` | Default value as string |

## Enum-Specific

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_underlying_type()` | `const std::string&` | Enum underlying type |
| `get_enum_value()` | `std::int64_t` | Enum constant value |

## Documentation Comments

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_comment()` | `const std::string&` | Full documentation comment |
| `get_brief_comment()` | `const std::string&` | Brief comment (first line/sentence) |

## Tags (Metadata Attributes)

Tags are C++ attributes like `[[xccmeta::reflect]]` that can be used for metadata.

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_tags()` | `const std::vector<tag>&` | All tags on this node |
| `has_tag(name)` | `bool` | Check if tag exists |
| `has_tags(names)` | `bool` | Check if any tag exists |
| `find_tag(name)` | `std::optional<tag>` | Find a specific tag |
| `find_tags(names)` | `std::vector<tag>` | Find all matching tags |
| `get_parent_tags()` | `std::vector<tag>` | Tags from parent nodes |
| `get_all_tags()` | `std::vector<tag>` | Own + parent tags combined |

### Example

```cpp
if (node->has_tag("xccmeta::reflect")) {
    auto tag = node->find_tag("xccmeta::reflect");
    if (tag) {
        for (const auto& arg : tag->get_args()) {
            std::cout << "Arg: " << arg << "\n";
        }
    }
}
```

## Tree Structure

### Basic Navigation

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_parent()` | `node_ptr` | Parent node (may be null) |
| `get_children()` | `const std::vector<node_ptr>&` | Direct children |

### Finding Children

```cpp
// Find first child matching predicate
template <typename Predicate>
node_ptr find_child(Predicate pred) const;

// Find all children matching predicate
template <typename Predicate>
std::vector<node_ptr> find_children(Predicate pred) const;

// Find all descendants (recursive) matching predicate
template <typename Predicate>
std::vector<node_ptr> find_descendants(Predicate pred) const;

// Get children by kind
std::vector<node_ptr> get_children_by_kind(kind k) const;

// Find child by name
node_ptr find_child_by_name(const std::string& name) const;
```

### Tag-Based Queries

```cpp
// Get children with a specific tag
std::vector<node_ptr> get_children_by_tag(const std::string& tag_name) const;
std::vector<node_ptr> get_children_by_tags(const std::vector<std::string>& tag_names) const;

// Get children without a specific tag
std::vector<node_ptr> get_children_without_tag(const std::string& tag_name) const;
std::vector<node_ptr> get_children_without_tags(const std::vector<std::string>& tag_names) const;

// Find first child with/without tag
node_ptr find_child_with_tag(const std::string& tag_name) const;
node_ptr find_child_without_tag(const std::string& tag_name) const;
```

### Example

```cpp
// Find all struct children
auto structs = node->get_children_by_kind(xccmeta::node::kind::struct_decl);

// Find all children with the "serialize" tag
auto serializable = node->get_children_by_tag("xccmeta::serialize");

// Find all class descendants recursively
auto all_classes = node->find_descendants([](const xccmeta::node_ptr& n) {
    return n->get_kind() == xccmeta::node::kind::class_decl;
});
```

## Convenience Queries

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_type_decl()` | `bool` | Is class/struct/union/enum/typedef? |
| `is_record_decl()` | `bool` | Is class/struct/union? |
| `is_callable()` | `bool` | Is function/method/constructor? |
| `get_bases()` | `std::vector<node_ptr>` | Base classes (for records) |
| `get_methods()` | `std::vector<node_ptr>` | Methods (for records) |
| `get_fields()` | `std::vector<node_ptr>` | Fields (for records) |
| `get_parameters()` | `std::vector<node_ptr>` | Parameters (for functions) |
| `get_enum_constants()` | `std::vector<node_ptr>` | Enum constants (for enums) |

### Example

```cpp
if (node->is_record_decl()) {
    std::cout << "Class: " << node->get_name() << "\n";
    
    for (const auto& base : node->get_bases()) {
        std::cout << "  Inherits: " << base->get_type().get_spelling() << "\n";
    }
    
    for (const auto& field : node->get_fields()) {
        std::cout << "  Field: " << field->get_name() << "\n";
    }
    
    for (const auto& method : node->get_methods()) {
        std::cout << "  Method: " << method->get_name() << "()\n";
    }
}
```

## Utility Functions

```cpp
// Convert enum to string
const char* access_specifier_to_string(access_specifier a);
const char* storage_class_to_string(storage_class sc);
const char* node::kind_to_string(kind k);
```

## Complete Example

```cpp
#include <xccmeta.hpp>
#include <iostream>

void print_node(const xccmeta::node_ptr& node, int indent = 0) {
    std::string prefix(indent * 2, ' ');
    
    std::cout << prefix << node->get_kind_name() << ": " << node->get_name();
    
    if (node->has_tag("xccmeta::reflect")) {
        std::cout << " [REFLECTED]";
    }
    
    std::cout << "\n";
    
    if (node->is_record_decl()) {
        for (const auto& field : node->get_fields()) {
            std::cout << prefix << "  - " << field->get_name() 
                      << ": " << field->get_type().get_spelling() << "\n";
        }
    }
    
    for (const auto& child : node->get_children()) {
        if (child->get_kind() != xccmeta::node::kind::field_decl) {
            print_node(child, indent + 1);
        }
    }
}

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        namespace game {
            struct [[xccmeta::reflect]] Vector3 {
                float x, y, z;
            };
            
            class [[xccmeta::reflect]] Entity {
            public:
                int id;
                Vector3 position;
                virtual void update() = 0;
            };
        }
    )", xccmeta::compile_args::modern_cxx());
    
    print_node(ast);
    return 0;
}
```

## See Also

- [Parser](parser.md) - Creating AST nodes
- [Type Info](type-info.md) - Type information details
- [Tags](tags.md) - Metadata attributes
- [Filter](filter.md) - Filtering nodes
