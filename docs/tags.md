# Tags

Tags are metadata annotations extracted from C++ attributes. They enable metadata-driven code generation and reflection systems.

## Header

```cpp
#include <xccmeta/xccmeta_tags.hpp>
```

## Overview

Tags correspond to C++11 attributes in the form `[[namespace::name(args...)]]`. xccmeta extracts these attributes and makes them available as `tag` objects that you can query and use for code generation.

## Syntax

```cpp
// Basic tag
[[xccmeta::reflect]]

// Tag with arguments
[[xccmeta::serialize("json", "xml")]]

// Multiple tags
[[xccmeta::reflect, xccmeta::component("physics")]]

// On different declarations
struct [[xccmeta::reflect]] MyStruct { };

class MyClass {
    [[xccmeta::property]] int value;
    [[xccmeta::rpc]] void remoteMethod();
};
```

## Class Reference

### Constructors

```cpp
tag();  // Default constructor
tag(const std::string& name, const std::vector<std::string>& args);
```

### Static Methods

#### parse

```cpp
static tag parse(const std::string& to_parse);
```

Parse a tag from a string (excluding the `[[` and `]]` brackets).

**Example:**
```cpp
auto t = xccmeta::tag::parse("xccmeta::reflect(arg1, arg2)");
// t.get_name() == "xccmeta::reflect"
// t.get_args() == {"arg1", "arg2"}
```

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_name()` | `const std::string&` | Tag name (e.g., "xccmeta::reflect") |
| `get_args()` | `const std::vector<std::string>&` | Tag arguments |
| `get_args_combined()` | `std::string` | Arguments as comma-separated string |
| `get_full()` | `std::string` | Full representation (name + args) |

### Example

```cpp
xccmeta::tag t("xccmeta::serialize", {"json", "binary"});

t.get_name();          // "xccmeta::serialize"
t.get_args();          // {"json", "binary"}
t.get_args_combined(); // "json, binary"
t.get_full();          // "xccmeta::serialize(json, binary)"
```

## Using Tags with Nodes

### Checking for Tags

```cpp
// Check if node has a specific tag
if (node->has_tag("xccmeta::reflect")) {
    // Node is marked for reflection
}

// Check if node has any of multiple tags
if (node->has_tags({"xccmeta::reflect", "xccmeta::serialize"})) {
    // Node has at least one of these tags
}
```

### Finding Tags

```cpp
// Get all tags
const auto& tags = node->get_tags();

// Find a specific tag
auto tag = node->find_tag("xccmeta::property");
if (tag) {
    std::cout << "Found property tag with args: " << tag->get_args_combined() << "\n";
}

// Find multiple tags
auto found = node->find_tags({"xccmeta::get", "xccmeta::set"});
```

### Parent Tags

Tags can be inherited from parent nodes:

```cpp
// Get tags from parent nodes (walking up the tree)
auto parent_tags = node->get_parent_tags();

// Get all tags (own + inherited from parents)
auto all_tags = node->get_all_tags();
```

### Tag-Based Child Queries

```cpp
// Get all children with a specific tag
auto reflected_fields = node->get_children_by_tag("xccmeta::reflect");

// Get all children with any of multiple tags
auto properties = node->get_children_by_tags({"xccmeta::get", "xccmeta::set"});

// Get all children WITHOUT a specific tag
auto non_serialized = node->get_children_without_tag("xccmeta::no_serialize");

// Find first child with/without tag
auto first_property = node->find_child_with_tag("xccmeta::property");
auto first_regular = node->find_child_without_tag("xccmeta::internal");
```

## Tag Naming Conventions

While you can use any attribute name, common conventions include:

```cpp
// Namespaced tags (recommended)
[[mylib::reflect]]
[[mylib::serialize]]
[[mylib::component("name")]]

// Short tags (may conflict with other libraries)
[[reflect]]
[[property]]
```

## Common Use Cases

### Reflection System

```cpp
// Mark types for reflection
struct [[mylib::reflect]] Player {
    [[mylib::replicated]] int health;
    [[mylib::replicated]] int score;
    int internal_counter;  // Not replicated
};

// Find reflected types
auto reflected = ast->find_descendants([](const auto& n) {
    return n->has_tag("mylib::reflect");
});

for (const auto& type : reflected) {
    auto replicated_fields = type->get_children_by_tag("mylib::replicated");
    // Generate replication code...
}
```

### Serialization

```cpp
struct [[mylib::serialize("json")]] Config {
    [[mylib::field("user_name")]] std::string name;
    [[mylib::field("max_count")]] int count;
    [[mylib::no_serialize]] std::string cache;
};

// Process serialization
auto tag = type->find_tag("mylib::serialize");
if (tag) {
    std::string format = tag->get_args().empty() ? "binary" : tag->get_args()[0];
    
    auto fields = type->get_children_by_tag("mylib::field");
    auto excluded = type->get_children_by_tag("mylib::no_serialize");
    // Generate serialization code...
}
```

### RPC/Networking

```cpp
class [[mylib::rpc_class]] GameService {
public:
    [[mylib::rpc("reliable")]]
    void spawnEntity(int type, float x, float y);
    
    [[mylib::rpc("unreliable")]]
    void updatePosition(int id, float x, float y, float z);
    
    void internalMethod();  // Not an RPC
};

// Find RPC methods
for (const auto& method : class_node->get_methods()) {
    auto rpc_tag = method->find_tag("mylib::rpc");
    if (rpc_tag) {
        std::string reliability = rpc_tag->get_args().empty() 
            ? "reliable" 
            : rpc_tag->get_args()[0];
        // Generate RPC stub...
    }
}
```

### Component Systems

```cpp
struct [[mylib::component("Transform")]] Transform {
    [[mylib::editable]] float x, y, z;
    [[mylib::editable, mylib::range(0, 360)]] float rotation;
};

// Extract component info
auto comp_tag = node->find_tag("mylib::component");
std::string comp_name = comp_tag ? comp_tag->get_args()[0] : node->get_name();

for (const auto& field : node->get_fields()) {
    if (field->has_tag("mylib::editable")) {
        auto range = field->find_tag("mylib::range");
        // Generate editor UI...
    }
}
```

## Complete Example

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        namespace game {
            struct [[reflect]] Vector3 {
                [[serialize]] float x;
                [[serialize]] float y;
                [[serialize]] float z;
            };
            
            class [[reflect, component("Entity")]] Entity {
            public:
                [[serialize, replicate]] int id;
                [[serialize]] Vector3 position;
                [[no_serialize]] int internal_state;
                
                [[rpc("reliable")]] void spawn();
                [[rpc("unreliable")]] void move(float dx, float dy);
                void update();  // Not an RPC
            };
        }
    )", xccmeta::compile_args::modern_cxx());
    
    // Find all reflected types
    auto reflected = ast->find_descendants([](const auto& n) {
        return n->has_tag("reflect");
    });
    
    for (const auto& type : reflected) {
        std::cout << "=== " << type->get_name() << " ===\n";
        
        // Check for component tag
        if (auto comp = type->find_tag("component")) {
            std::cout << "Component: " << comp->get_args_combined() << "\n";
        }
        
        // Serialized fields
        std::cout << "Serialized fields:\n";
        for (const auto& field : type->get_children_by_tag("serialize")) {
            std::cout << "  - " << field->get_name();
            if (field->has_tag("replicate")) {
                std::cout << " (replicated)";
            }
            std::cout << "\n";
        }
        
        // Non-serialized fields
        auto excluded = type->get_children_by_tag("no_serialize");
        if (!excluded.empty()) {
            std::cout << "Excluded from serialization:\n";
            for (const auto& field : excluded) {
                std::cout << "  - " << field->get_name() << "\n";
            }
        }
        
        // RPC methods
        if (type->is_record_decl()) {
            std::cout << "RPC methods:\n";
            for (const auto& method : type->get_methods()) {
                if (auto rpc = method->find_tag("rpc")) {
                    std::cout << "  - " << method->get_name() 
                              << " (" << rpc->get_args_combined() << ")\n";
                }
            }
        }
        
        std::cout << "\n";
    }
    
    return 0;
}
```

## See Also

- [Node](node.md) - Tag queries on nodes
- [Filter](filter.md) - Filtering by tags
- [Generator](generator.md) - Generating code from tags
