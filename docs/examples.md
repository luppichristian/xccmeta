# Examples

This page provides complete, working examples demonstrating various xccmeta use cases.

## Basic Parsing

### Parse and Explore AST

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        namespace game {
            struct Vector3 {
                float x, y, z;
            };
            
            class Entity {
            public:
                int id;
                Vector3 position;
                
                virtual void update() = 0;
                void render();
            };
        }
    )", xccmeta::compile_args::modern_cxx());
    
    // Print AST structure
    std::function<void(const xccmeta::node_ptr&, int)> print_node;
    print_node = [&](const xccmeta::node_ptr& node, int depth) {
        std::string indent(depth * 2, ' ');
        std::cout << indent << node->get_kind_name();
        if (!node->get_name().empty()) {
            std::cout << ": " << node->get_name();
        }
        std::cout << "\n";
        
        for (const auto& child : node->get_children()) {
            print_node(child, depth + 1);
        }
    };
    
    print_node(ast, 0);
    return 0;
}
```

## Reflection System

### Complete Reflection Generator

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        struct [[reflect]] Player {
            [[property]] int health = 100;
            [[property]] int score = 0;
            [[property, readonly]] int id;
            
            [[rpc]] void takeDamage(int amount);
            [[rpc]] void heal(int amount);
        };
        
        struct [[reflect]] Enemy {
            [[property]] int health = 50;
            [[property]] int damage = 10;
            
            [[rpc]] void attack(int targetId);
        };
        
        struct Internal {
            // Not reflected
            int value;
        };
    )", xccmeta::compile_args::modern_cxx());
    
    // Find all reflected types
    auto reflected = ast->find_descendants([](const auto& n) {
        return n->has_tag("reflect") && n->is_record_decl();
    });
    
    std::cout << "=== Reflection Report ===\n\n";
    
    for (const auto& type : reflected) {
        std::cout << "Type: " << type->get_name() << "\n";
        
        // Properties
        auto properties = type->get_children_by_tag("property");
        if (!properties.empty()) {
            std::cout << "  Properties:\n";
            for (const auto& prop : properties) {
                std::cout << "    - " << prop->get_name() 
                          << " (" << prop->get_type().get_spelling() << ")";
                if (prop->has_tag("readonly")) {
                    std::cout << " [readonly]";
                }
                if (prop->has_default_value()) {
                    std::cout << " = " << prop->get_default_value();
                }
                std::cout << "\n";
            }
        }
        
        // RPC Methods
        auto rpcs = type->get_children_by_tag("rpc");
        if (!rpcs.empty()) {
            std::cout << "  RPC Methods:\n";
            for (const auto& rpc : rpcs) {
                std::cout << "    - " << rpc->get_name() << "(";
                auto params = rpc->get_parameters();
                for (size_t i = 0; i < params.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << params[i]->get_type().get_spelling() 
                              << " " << params[i]->get_name();
                }
                std::cout << ")\n";
            }
        }
        
        std::cout << "\n";
    }
    
    return 0;
}
```

## Serialization Code Generator

### Generate JSON Serializer

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        struct [[serialize("json")]] Config {
            [[field("user_name")]] std::string name;
            [[field("max_players")]] int maxPlayers = 8;
            [[field("debug_mode")]] bool debug = false;
            
            [[no_serialize]] std::string cacheData;
        };
    )", xccmeta::compile_args::modern_cxx());
    
    xccmeta::generator gen("config_serializer.generated.cpp");
    
    gen.out("// Auto-generated JSON serializer");
    gen.out("#include <nlohmann/json.hpp>");
    gen.out("#include \"config.hpp\"");
    gen.separator();
    
    auto types = ast->find_descendants([](const auto& n) {
        return n->has_tag("serialize");
    });
    
    for (const auto& type : types) {
        auto serialize_tag = type->find_tag("serialize");
        std::string format = serialize_tag->get_args().empty() 
            ? "binary" : serialize_tag->get_args()[0];
        
        if (format != "json") continue;
        
        gen.named_separator("Serializer for " + type->get_name());
        
        // to_json
        gen.out("void to_json(nlohmann::json& j, const " + type->get_name() + "& obj) {");
        gen.indent();
        gen.out("j = nlohmann::json{");
        gen.indent();
        
        auto fields = type->get_children_by_tag("field");
        for (size_t i = 0; i < fields.size(); ++i) {
            auto field_tag = fields[i]->find_tag("field");
            std::string json_name = field_tag->get_args().empty() 
                ? fields[i]->get_name() : field_tag->get_args()[0];
            
            std::string line = "{\"" + json_name + "\", obj." + fields[i]->get_name() + "}";
            if (i < fields.size() - 1) line += ",";
            gen.out(line);
        }
        
        gen.unindent();
        gen.out("};");
        gen.unindent();
        gen.out("}");
        gen.out("");
        
        // from_json
        gen.out("void from_json(const nlohmann::json& j, " + type->get_name() + "& obj) {");
        gen.indent();
        
        for (const auto& field : fields) {
            auto field_tag = field->find_tag("field");
            std::string json_name = field_tag->get_args().empty() 
                ? field->get_name() : field_tag->get_args()[0];
            
            gen.out("j.at(\"" + json_name + "\").get_to(obj." + field->get_name() + ");");
        }
        
        gen.unindent();
        gen.out("}");
        gen.out("");
    }
    
    gen.done();
    std::cout << "Generated serializer code\n";
    return 0;
}
```

## Entity Component System

### Component Registration

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        struct [[component("Transform")]] TransformComponent {
            [[editable]] float x = 0.0f;
            [[editable]] float y = 0.0f;
            [[editable, range(0, 360)]] float rotation = 0.0f;
        };
        
        struct [[component("Sprite")]] SpriteComponent {
            [[editable, asset("texture")]] std::string texture;
            [[editable, range(0, 1)]] float alpha = 1.0f;
        };
        
        struct [[component("Physics")]] PhysicsComponent {
            [[editable]] float mass = 1.0f;
            [[editable]] bool isStatic = false;
        };
    )", xccmeta::compile_args::modern_cxx());
    
    // Find all components
    auto components = ast->find_descendants([](const auto& n) {
        return n->has_tag("component");
    });
    
    xccmeta::generator gen("component_registry.generated.cpp");
    
    gen.out("// Auto-generated component registry");
    gen.out("#include \"ecs/component_registry.hpp\"");
    gen.separator();
    
    gen.out("void register_all_components() {");
    gen.indent();
    
    for (const auto& comp : components) {
        auto comp_tag = comp->find_tag("component");
        std::string comp_name = comp_tag->get_args().empty() 
            ? comp->get_name() : comp_tag->get_args()[0];
        
        gen.out("// " + comp_name);
        gen.out("ComponentRegistry::register_component<" + comp->get_name() + ">(");
        gen.indent();
        gen.out("\"" + comp_name + "\",");
        gen.out("{");
        gen.indent();
        
        auto editable_fields = comp->get_children_by_tag("editable");
        for (const auto& field : editable_fields) {
            std::string field_line = "{\"" + field->get_name() + "\", ";
            field_line += "offsetof(" + comp->get_name() + ", " + field->get_name() + ")";
            field_line += ", TypeInfo<" + field->get_type().get_spelling() + ">::id";
            
            // Check for range attribute
            if (auto range_tag = field->find_tag("range")) {
                field_line += ", Range{" + range_tag->get_args_combined() + "}";
            }
            
            // Check for asset attribute
            if (auto asset_tag = field->find_tag("asset")) {
                field_line += ", AssetType::\"" + asset_tag->get_args()[0] + "\"";
            }
            
            field_line += "},";
            gen.out(field_line);
        }
        
        gen.unindent();
        gen.out("}");
        gen.unindent();
        gen.out(");");
        gen.out("");
    }
    
    gen.unindent();
    gen.out("}");
    
    gen.done();
    std::cout << "Generated component registry\n";
    return 0;
}
```

## Enum Reflection

### Generate Enum Utilities

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        enum class [[reflect]] Color {
            Red = 0,
            Green = 1,
            Blue = 2,
            Yellow = 3
        };
        
        enum class [[reflect]] Direction {
            North,
            East,
            South,
            West
        };
    )", xccmeta::compile_args::modern_cxx());
    
    auto enums = ast->find_descendants([](const auto& n) {
        return n->get_kind() == xccmeta::node::kind::enum_decl && n->has_tag("reflect");
    });
    
    xccmeta::generator gen("enum_utils.generated.hpp");
    
    gen.out("#pragma once");
    gen.out("#include <string_view>");
    gen.out("#include <optional>");
    gen.separator();
    
    for (const auto& e : enums) {
        auto constants = e->get_enum_constants();
        
        gen.named_separator(e->get_name() + " utilities");
        
        // to_string
        gen.out("constexpr std::string_view to_string(" + e->get_name() + " value) {");
        gen.indent();
        gen.out("switch (value) {");
        gen.indent();
        
        for (const auto& c : constants) {
            gen.out("case " + e->get_name() + "::" + c->get_name() + 
                   ": return \"" + c->get_name() + "\";");
        }
        gen.out("default: return \"unknown\";");
        
        gen.unindent();
        gen.out("}");
        gen.unindent();
        gen.out("}");
        gen.out("");
        
        // from_string
        gen.out("constexpr std::optional<" + e->get_name() + "> " + 
               e->get_name() + "_from_string(std::string_view str) {");
        gen.indent();
        
        for (const auto& c : constants) {
            gen.out("if (str == \"" + c->get_name() + "\") return " + 
                   e->get_name() + "::" + c->get_name() + ";");
        }
        gen.out("return std::nullopt;");
        
        gen.unindent();
        gen.out("}");
        gen.out("");
        
        // count
        gen.out("constexpr size_t " + e->get_name() + "_count = " + 
               std::to_string(constants.size()) + ";");
        gen.out("");
    }
    
    gen.done();
    std::cout << "Generated enum utilities\n";
    return 0;
}
```

## Multi-File Processing

### Process Entire Project

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    // Import all headers
    xccmeta::importer headers("include/**/*.hpp");
    
    std::cout << "Found " << headers.get_files().size() << " header files\n";
    
    // Configure compilation
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    args.add_include_path("include");
    args.add_include_path("third_party");
    args.define("GENERATING_REFLECTION");
    
    // Parse and merge all files
    xccmeta::parser parser;
    xccmeta::node_ptr merged = nullptr;
    
    for (const auto& file : headers.get_files()) {
        std::cout << "Processing: " << file.get_path().filename() << "\n";
        
        std::string content = file.read();
        auto ast = parser.parse(content, args);
        
        if (merged) {
            merged = parser.merge(merged, ast, args);
        } else {
            merged = ast;
        }
    }
    
    if (!merged) {
        std::cerr << "No files parsed\n";
        return 1;
    }
    
    // Collect statistics
    auto all_types = merged->find_descendants([](const auto& n) {
        return n->is_record_decl();
    });
    
    auto reflected = merged->find_descendants([](const auto& n) {
        return n->has_tag("reflect") && n->is_record_decl();
    });
    
    std::cout << "\n=== Project Statistics ===\n";
    std::cout << "Total types: " << all_types.size() << "\n";
    std::cout << "Reflected types: " << reflected.size() << "\n";
    
    // Generate reflection data
    xccmeta::generator gen("generated/reflection_data.cpp");
    // ... generate code ...
    gen.done();
    
    return 0;
}
```

## Script Bindings

### Generate Lua Bindings

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        class [[lua_class("Vec3")]] Vector3 {
        public:
            float x, y, z;
            
            [[lua_method]] float length() const;
            [[lua_method]] Vector3 normalized() const;
            [[lua_method]] static Vector3 zero();
            
            [[lua_operator("+")]] Vector3 operator+(const Vector3& other) const;
            [[lua_operator("*")]] Vector3 operator*(float scalar) const;
        };
    )", xccmeta::compile_args::modern_cxx());
    
    auto lua_classes = ast->find_descendants([](const auto& n) {
        return n->has_tag("lua_class");
    });
    
    xccmeta::generator gen("lua_bindings.generated.cpp");
    
    gen.out("// Auto-generated Lua bindings");
    gen.out("#include <sol/sol.hpp>");
    gen.separator();
    
    gen.out("void register_lua_bindings(sol::state& lua) {");
    gen.indent();
    
    for (const auto& cls : lua_classes) {
        auto lua_tag = cls->find_tag("lua_class");
        std::string lua_name = lua_tag->get_args().empty() 
            ? cls->get_name() : lua_tag->get_args()[0];
        
        gen.out("lua.new_usertype<" + cls->get_name() + ">(\"" + lua_name + "\",");
        gen.indent();
        
        // Properties (public fields)
        for (const auto& field : cls->get_fields()) {
            if (field->get_access() == xccmeta::access_specifier::public_) {
                gen.out("\"" + field->get_name() + "\", &" + 
                       cls->get_name() + "::" + field->get_name() + ",");
            }
        }
        
        // Methods
        auto methods = cls->get_children_by_tag("lua_method");
        for (const auto& method : methods) {
            gen.out("\"" + method->get_name() + "\", &" + 
                   cls->get_name() + "::" + method->get_name() + ",");
        }
        
        // Operators
        auto operators = cls->get_children_by_tag("lua_operator");
        for (const auto& op : operators) {
            auto op_tag = op->find_tag("lua_operator");
            std::string op_str = op_tag->get_args()[0];
            std::string meta = "sol::meta_function::";
            
            if (op_str == "+") meta += "addition";
            else if (op_str == "-") meta += "subtraction";
            else if (op_str == "*") meta += "multiplication";
            else if (op_str == "/") meta += "division";
            
            gen.out(meta + ", &" + cls->get_name() + "::" + op->get_name() + ",");
        }
        
        gen.unindent();
        gen.out(");");
        gen.out("");
    }
    
    gen.unindent();
    gen.out("}");
    
    gen.done();
    std::cout << "Generated Lua bindings\n";
    return 0;
}
```

## See Also

- [Quick Start](quickstart.md) - Getting started guide
- [API Reference](api-reference.md) - Complete API documentation
