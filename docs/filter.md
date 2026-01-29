# Filter

The `filter` class provides a way to collect and filter AST nodes based on various criteria including node kind, tags, and parent/child relationships.

## Header

```cpp
#include <xccmeta/xccmeta_filter.hpp>
```

## Overview

The filter maintains a list of unique nodes (identified by USR - Unified Symbol Resolution) and can automatically include or exclude nodes based on configurable criteria. It's useful for:

- Collecting types marked for reflection
- Filtering nodes by kind (only classes, only structs, etc.)
- Including/excluding nodes based on tags
- Managing child and parent node inclusion

## Configuration

### config Structure

```cpp
struct config {
    enum node_inclusion {
        exclude,             // Only include the specified node
        include,             // Include direct children of the specified node
        include_recursively  // Include all descendant nodes
    };
    
    // Allowed kinds of types to include
    std::vector<node::kind> allowed_kinds;
    
    // If a node has a tag name in this list, it will be included
    std::vector<std::string> grab_tag_names;
    
    // If a node has a tag name in this list, it will be excluded
    std::vector<std::string> avoid_tag_names;
    
    // Whether to include child nodes
    node_inclusion child_node_inclusion = node_inclusion::exclude;
    
    // Whether to include parent nodes
    node_inclusion parent_node_inclusion = node_inclusion::exclude;
};
```

### Configuration Options

| Option | Description |
|--------|-------------|
| `allowed_kinds` | Only nodes of these kinds will be accepted |
| `grab_tag_names` | Include nodes with any of these tags |
| `avoid_tag_names` | Exclude nodes with any of these tags |
| `child_node_inclusion` | How to handle children of added nodes |
| `parent_node_inclusion` | How to handle parents of added nodes |

## Class Reference

### Constructor

```cpp
filter(const config& cfg = {});
```

Create a filter with the given configuration. Default configuration accepts all nodes.

### Methods

#### add

```cpp
bool add(const node_ptr& type);
```

Add a node to the filter. Returns `true` if added, `false` if already exists or doesn't match criteria.

#### remove

```cpp
bool remove(const node_ptr& type);
```

Remove a node from the filter. Returns `true` if removed, `false` if not found.

#### contains

```cpp
bool contains(const node_ptr& type) const;
```

Check if a node is in the filter (by USR).

#### clean

```cpp
filter& clean();
```

Remove all nodes that don't meet the current criteria. Returns `*this` for chaining.

#### clear

```cpp
filter& clear();
```

Remove all nodes from the filter.

#### size / empty

```cpp
std::size_t size() const;
bool empty() const;
```

Get the number of nodes or check if empty.

#### get_types

```cpp
const std::vector<node_ptr>& get_types() const;
```

Get all nodes in the filter.

#### get_config

```cpp
const config& get_config() const;
```

Get the current configuration.

#### is_valid_type

```cpp
bool is_valid_type(const node_ptr& type) const;
```

Check if a node is valid for this filter's configuration.

#### matches_config

```cpp
bool matches_config(const node_ptr& type) const;
```

Check if a node matches all config criteria.

### Iterator Support

The filter supports range-based for loops:

```cpp
xccmeta::filter f(cfg);
// ... add nodes ...

for (const auto& node : f) {
    // Process each node
}
```

## Basic Usage

### Collecting Specific Node Kinds

```cpp
xccmeta::filter::config cfg;
cfg.allowed_kinds = {
    xccmeta::node::kind::class_decl,
    xccmeta::node::kind::struct_decl
};

xccmeta::filter filter(cfg);

// Add nodes from AST
for (const auto& child : ast->get_children()) {
    filter.add(child);
}

// Iterate over filtered nodes
for (const auto& type : filter) {
    std::cout << "Found: " << type->get_name() << "\n";
}
```

### Filtering by Tags

```cpp
xccmeta::filter::config cfg;
cfg.grab_tag_names = {"xccmeta::reflect", "xccmeta::serialize"};
cfg.avoid_tag_names = {"xccmeta::ignore", "xccmeta::internal"};

xccmeta::filter filter(cfg);

// Add all descendants
auto all_nodes = ast->find_descendants([](const auto&) { return true; });
for (const auto& node : all_nodes) {
    filter.add(node);
}

// Only nodes with reflect/serialize tags (and without ignore/internal) remain
```

### Including Children

```cpp
xccmeta::filter::config cfg;
cfg.allowed_kinds = {xccmeta::node::kind::struct_decl};
cfg.grab_tag_names = {"xccmeta::reflect"};
cfg.child_node_inclusion = xccmeta::filter::config::include;

xccmeta::filter filter(cfg);

// When adding a reflected struct, its direct children are also included
filter.add(reflected_struct);
```

### Recursive Child Inclusion

```cpp
xccmeta::filter::config cfg;
cfg.child_node_inclusion = xccmeta::filter::config::include_recursively;

// When adding a namespace, all nested declarations are included
filter.add(namespace_node);
```

## Complete Example

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        namespace game {
            struct [[xccmeta::reflect]] Player {
                int id;
                float health;
            };
            
            struct [[xccmeta::reflect]] Enemy {
                int id;
                int damage;
            };
            
            struct [[xccmeta::internal]] DebugInfo {
                int frame;
            };
            
            class Helper {
                // No tags, won't be included
            };
        }
    )", xccmeta::compile_args::modern_cxx());
    
    // Configure filter for reflected types
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {
        xccmeta::node::kind::class_decl,
        xccmeta::node::kind::struct_decl
    };
    cfg.grab_tag_names = {"xccmeta::reflect"};
    cfg.avoid_tag_names = {"xccmeta::internal"};
    cfg.child_node_inclusion = xccmeta::filter::config::include;
    
    xccmeta::filter filter(cfg);
    
    // Add all type declarations
    auto types = ast->find_descendants([](const auto& n) {
        return n->is_record_decl();
    });
    
    for (const auto& type : types) {
        bool added = filter.add(type);
        std::cout << type->get_name() << ": " 
                  << (added ? "added" : "rejected") << "\n";
    }
    
    std::cout << "\n=== Filtered Types ===\n";
    for (const auto& type : filter) {
        std::cout << type->get_name() << "\n";
        for (const auto& field : type->get_fields()) {
            std::cout << "  - " << field->get_name() << "\n";
        }
    }
    
    return 0;
}
```

**Output:**
```
Player: added
Enemy: added
DebugInfo: rejected
Helper: rejected

=== Filtered Types ===
Player
  - id
  - health
Enemy
  - id
  - damage
```

## Use Cases

### Reflection Code Generator

```cpp
xccmeta::filter::config cfg;
cfg.allowed_kinds = {
    xccmeta::node::kind::class_decl,
    xccmeta::node::kind::struct_decl,
    xccmeta::node::kind::enum_decl
};
cfg.grab_tag_names = {"reflect"};
cfg.child_node_inclusion = xccmeta::filter::config::include;

xccmeta::filter reflected_types(cfg);

// Populate from AST
populate_filter(ast, reflected_types);

// Generate reflection code
for (const auto& type : reflected_types) {
    generate_type_info(type);
}
```

### Serialization System

```cpp
xccmeta::filter::config cfg;
cfg.grab_tag_names = {"serialize", "persist"};
cfg.avoid_tag_names = {"no_serialize", "transient"};

xccmeta::filter serializable(cfg);
// ...
```

### Component Collection

```cpp
xccmeta::filter::config cfg;
cfg.grab_tag_names = {"component"};
cfg.child_node_inclusion = xccmeta::filter::config::include;

xccmeta::filter components(cfg);
// Collect all components and their fields
```

## See Also

- [Node](node.md) - Node structure and queries
- [Tags](tags.md) - Working with metadata tags
- [Parser](parser.md) - Creating AST to filter
