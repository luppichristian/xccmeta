# Quick Start

Get up and running with xccmeta in minutes.

## Basic Workflow

The typical xccmeta workflow consists of:

1. **Configure compile arguments** - Set up language standard, includes, defines
2. **Parse source code** - Convert C/C++ code into an AST
3. **Traverse/filter nodes** - Find the declarations you're interested in
4. **Extract metadata** - Read type information, tags, and other properties
5. **Generate output** - Use the generator to create output files

## Hello World Example

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    // Step 1: Configure compile arguments
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    
    // Step 2: Parse source code
    xccmeta::parser parser;
    auto root = parser.parse(R"(
        namespace myapp {
            struct Vector3 {
                float x, y, z;
            };
            
            class Entity {
            public:
                int id;
                Vector3 position;
                
                void update();
            };
        }
    )", args);
    
    // Step 3: Find all struct/class declarations
    auto types = root->find_descendants([](const xccmeta::node_ptr& n) {
        return n->is_record_decl();
    });
    
    // Step 4: Extract metadata
    for (const auto& type : types) {
        std::cout << "Found type: " << type->get_qualified_name() << "\n";
        
        for (const auto& field : type->get_fields()) {
            std::cout << "  Field: " << field->get_name() 
                      << " (" << field->get_type().get_spelling() << ")\n";
        }
    }
    
    return 0;
}
```

**Output:**
```
Found type: myapp::Vector3
  Field: x (float)
  Field: y (float)
  Field: z (float)
Found type: myapp::Entity
  Field: id (int)
  Field: position (Vector3)
```

## Using Tags for Reflection

Tags (C++ attributes) let you mark which types/members should be processed:

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    xccmeta::parser parser;
    
    auto root = parser.parse(R"(
        // Only this struct will be reflected
        struct [[xccmeta::reflect]] Player {
            [[xccmeta::serialize]] int health;
            [[xccmeta::serialize]] int score;
            int internal_state;  // Not serialized
        };
        
        // This struct won't be reflected
        struct InternalData {
            int value;
        };
    )", args);
    
    // Find types with the reflect tag
    auto reflected = root->find_descendants([](const xccmeta::node_ptr& n) {
        return n->has_tag("xccmeta::reflect");
    });
    
    for (const auto& type : reflected) {
        std::cout << "Reflected type: " << type->get_name() << "\n";
        
        // Find fields with serialize tag
        auto serialized_fields = type->get_children_by_tag("xccmeta::serialize");
        for (const auto& field : serialized_fields) {
            std::cout << "  Serialized field: " << field->get_name() << "\n";
        }
    }
    
    return 0;
}
```

**Output:**
```
Reflected type: Player
  Serialized field: health
  Serialized field: score
```

## Importing Files

Process multiple source files using wildcards:

```cpp
#include <xccmeta.hpp>

int main() {
    // Import all header files in a directory
    xccmeta::importer importer("src/**/*.hpp");
    
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    args.add_include_path("include");
    
    xccmeta::parser parser;
    xccmeta::node_ptr merged_ast = nullptr;
    
    for (const auto& file : importer.get_files()) {
        std::string content = file.read();
        auto ast = parser.parse(content, args);
        
        if (merged_ast) {
            merged_ast = parser.merge(merged_ast, ast, args);
        } else {
            merged_ast = ast;
        }
    }
    
    // Now merged_ast contains all parsed declarations
    return 0;
}
```

## Generating Output

Use the generator to write processed metadata:

```cpp
#include <xccmeta.hpp>

int main() {
    // ... parse code as shown above ...
    
    xccmeta::generator gen("output/reflection_data.generated.cpp");
    
    gen.out("// Auto-generated reflection data");
    gen.out("#include \"reflection.hpp\"");
    gen.separator();
    
    for (const auto& type : reflected_types) {
        gen.named_separator(type->get_name());
        gen.out("REGISTER_TYPE(" + type->get_qualified_name() + ");");
        
        gen.indent();
        for (const auto& field : type->get_fields()) {
            gen.out("REGISTER_FIELD(" + field->get_name() + ", " 
                    + field->get_type().get_spelling() + ");");
        }
        gen.unindent();
    }
    
    gen.done();
    
    return 0;
}
```

## Using Filters

For more complex filtering scenarios:

```cpp
#include <xccmeta.hpp>

int main() {
    // ... parse code ...
    
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {
        xccmeta::node::kind::class_decl,
        xccmeta::node::kind::struct_decl
    };
    cfg.grab_tag_names = {"xccmeta::reflect", "xccmeta::component"};
    cfg.avoid_tag_names = {"xccmeta::ignore"};
    cfg.child_node_inclusion = xccmeta::filter::config::include;
    
    xccmeta::filter filter(cfg);
    
    // Add nodes from the AST
    for (const auto& child : root->get_children()) {
        filter.add(child);
    }
    
    // Clean removes nodes that don't match criteria
    filter.clean();
    
    for (const auto& type : filter.get_types()) {
        // Process filtered types
    }
    
    return 0;
}
```

## Next Steps

- [Parser Documentation](parser.md) - Learn about AST parsing in detail
- [Node Documentation](node.md) - Understand the AST structure
- [Tags Documentation](tags.md) - Master metadata annotations
- [Examples](examples.md) - More complete examples
