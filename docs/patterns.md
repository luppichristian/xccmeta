# Common Patterns

Recipes for frequent xccmeta tasks. Copy-paste starting points for typical workflows.

## Pattern: Basic Enum-to-String Generator

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
  // 1. Parse
  xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
  xccmeta::parser parser;
  auto ast = parser.parse(source_code, args);

  if (!ast) {
    std::cerr << "Parse failed\n";
    return 1;
  }

  // 2. Find tagged enums
  auto enums = ast->find_descendants([](auto& n) {
    return n->get_kind() == xccmeta::node::kind::enum_decl
        && n->has_tag("reflect");
  });

  // 3. Generate
  xccmeta::generator gen("enum_strings.hpp");
  gen.out("#pragma once");
  gen.out("#include <string>");
  gen.out("");

  for (auto& e : enums) {
    gen.out("inline std::string to_string(" + e->get_qualified_name() + " val) {");
    gen.indent();
    gen.out("switch (val) {");
    gen.indent();

    for (auto& constant : e->get_enum_constants()) {
      auto name = constant->get_qualified_name();
      gen.out("case " + name + ": return \"" + constant->get_name() + "\";");
    }

    gen.unindent();
    gen.out("}");
    gen.out("return \"<unknown>\";");
    gen.unindent();
    gen.out("}");
    gen.out("");
  }

  return gen.done() ? 0 : 1;
}
```

## Pattern: Multi-File Type Collection

```cpp
#include <xccmeta.hpp>

int main() {
  // 1. Import files
  xccmeta::importer headers("include/**/*.hpp");

  // 2. Setup filter
  xccmeta::filter::config cfg;
  cfg.allowed_kinds = {
    xccmeta::node::kind::struct_decl,
    xccmeta::node::kind::class_decl
  };
  cfg.grab_tag_names = {"serialize"};

  xccmeta::filter types(cfg);

  // 3. Parse all files, collect types
  xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
  args.add_include_path("include");

  xccmeta::parser parser;
  for (auto& file : headers.get_files()) {
    auto content = file.read();
    auto ast = parser.parse(content, args);

    if (ast) {
      for (auto& child : ast->get_children()) {
        types.add(child);
      }
    }
  }

  types.clean(); // Remove non-matching nodes

  // 4. Generate from deduplicated types
  // ... use types.get_types()
}
```

## Pattern: Field Iteration with Type Checks

```cpp
for (auto& field : struct_node->get_fields()) {
  // Skip private fields
  if (field->get_access() != xccmeta::access_specifier::public_) {
    continue;
  }

  auto type = field->get_type();
  auto name = field->get_name();

  // Handle different type categories
  if (type.is_array()) {
    auto elem_type = type.get_array_element_type();
    auto count = type.get_array_size();

    if (count == -1) {
      gen.warn("Unsized array: " + name, field);
      continue;
    }

    gen.out("// Array: " + name + "[" + std::to_string(count) + "]");

  } else if (type.is_pointer()) {
    auto pointee = type.get_pointee_type();
    gen.out("// Pointer: " + name + " -> " + pointee);

  } else if (type.is_builtin()) {
    gen.out("// Builtin: " + name + " (" + type.get_spelling() + ")");

  } else {
    // Custom type
    gen.out("// Custom: " + name + " (" + type.get_canonical() + ")");
  }
}
```

## Pattern: Method Filtering

```cpp
// Find all public non-deleted constructors
auto ctors = class_node->find_children([](auto& n) {
  return n->get_kind() == xccmeta::node::kind::constructor_decl
      && n->get_access() == xccmeta::access_specifier::public_
      && !n->is_deleted()
      && !n->is_defaulted();
});

// Find all const methods
auto const_methods = class_node->find_children([](auto& n) {
  return n->get_kind() == xccmeta::node::kind::method_decl
      && n->is_const_method();
});

// Find virtual methods
auto virtual_methods = class_node->find_children([](auto& n) {
  return n->get_kind() == xccmeta::node::kind::method_decl
      && n->is_virtual();
});
```

## Pattern: Tag-Based Configuration

```cpp
for (auto& type : types) {
  // Check for exclusion tag
  if (type->has_tag("no_codegen")) {
    continue;
  }

  // Get optional config from tag
  bool binary_mode = false;
  if (auto tag = type->find_tag("serialize")) {
    auto& args = tag->get_args();
    if (!args.empty() && args[0] == "binary") {
      binary_mode = true;
    }
  }

  // Use tag arguments for generation
  if (binary_mode) {
    generate_binary_serializer(type);
  } else {
    generate_text_serializer(type);
  }
}
```

## Pattern: Handling Inheritance

```cpp
void process_class(const xccmeta::node_ptr& cls) {
  // Get base classes
  auto bases = cls->get_bases();

  if (!bases.empty()) {
    gen.out("// Inherits from:");
    for (auto& base : bases) {
      auto access = xccmeta::access_specifier_to_string(base->get_access());
      auto name = base->get_name();

      std::string mods;
      if (base->is_virtual_base()) {
        mods = "virtual ";
      }

      gen.out("//   " + mods + access + " " + name);
    }
  }

  // Process own fields (not inherited)
  for (auto& field : cls->get_fields()) {
    // ...
  }
}
```

## Pattern: Error-Resilient Generator

```cpp
int main() {
  try {
    xccmeta::importer imp("src/**/*.hpp");

    if (imp.get_files().empty()) {
      std::cerr << "No files matched pattern\n";
      return 1;
    }

    xccmeta::parser parser;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    std::vector<xccmeta::node_ptr> all_asts;

    for (auto& file : imp.get_files()) {
      std::cout << "Parsing " << file.get_path() << "...\n";

      auto content = file.read();
      auto ast = parser.parse(content, args);

      if (ast) {
        all_asts.push_back(ast);
      } else {
        std::cerr << "Warning: Failed to parse " << file.get_path() << "\n";
        // Continue with other files
      }
    }

    if (all_asts.empty()) {
      std::cerr << "No files parsed successfully\n";
      return 1;
    }

    // Generate from successfully parsed ASTs
    // ...

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
```

## Pattern: Compile Argument Customization

```cpp
// Platform-specific parsing
xccmeta::compile_args args;
args.set_standard(xccmeta::language_standard::cxx20);

#ifdef _WIN32
  args.define("PLATFORM_WINDOWS")
      .add_include_path("C:/Program Files/SDK/include");
#else
  args.define("PLATFORM_LINUX")
      .add_include_path("/usr/local/include");
#endif

// Debug vs Release
#ifndef NDEBUG
  args.define("DEBUG_BUILD");
#endif

// Custom macros
args.define("PROJECT_VERSION", "1.2.3")
    .define("FEATURE_ENABLED", "1");

auto ast = parser.parse(source, args);
```

## Pattern: Incremental Code Generation

```cpp
// Generate header
{
  xccmeta::generator gen("types.generated.hpp");
  gen.out("#pragma once");

  for (auto& type : types) {
    gen.out("struct " + type->get_name() + "_Meta;");
  }
}

// Generate implementation
{
  xccmeta::generator gen("types.generated.cpp");
  gen.out("#include \"types.generated.hpp\"");
  gen.out("#include \"original_types.hpp\"");
  gen.out("");

  for (auto& type : types) {
    gen.out("struct " + type->get_name() + "_Meta {");
    gen.indent();
    // Implementation
    gen.unindent();
    gen.out("};");
  }
}
```

## Pattern: Location-Based Diagnostics

```cpp
for (auto& node : nodes) {
  auto loc = node->get_location();

  if (!validate(node)) {
    std::cerr << loc.to_string() << ": error: validation failed for "
              << node->get_name() << "\n";
  }

  if (needs_warning(node)) {
    gen.warn("Consider using std::optional", node);
  }
}
```

## Pattern: Parent Tag Inheritance

```cpp
/// @namespace_config(version=2)
namespace app {
  /// @reflect
  struct Data {
    int value;
  };
}

// Query parent tags
auto struct_node = /* Data struct */;
auto parent_tags = struct_node->get_parent_tags();

for (auto& tag : parent_tags) {
  if (tag.get_name() == "namespace_config") {
    auto version = tag.get_args()[0]; // "version=2"
    // Use namespace-level config
  }
}
```

## Anti-Patterns

**Don't preprocess before parsing:**
```cpp
// WRONG
auto preprocessed = xccmeta::preprocessor(file, args).get_preprocessed_content()[0];
auto ast = parser.parse(preprocessed, args);

// CORRECT
auto ast = parser.parse(file.read(), args);
```

**Don't assume type size is available:**
```cpp
// WRONG
auto size = field->get_type().get_size_bytes();
gen.out("char buffer[" + std::to_string(size) + "];");

// CORRECT
auto size = field->get_type().get_size_bytes();
if (size != -1) {
  gen.out("char buffer[" + std::to_string(size) + "];");
} else {
  gen.warn("Type size unavailable", field);
}
```

**Don't modify AST after parsing:**
```cpp
// WRONG - no public setters
node->set_name("new_name"); // Compile error

// CORRECT - use metadata as-is
auto name = node->get_name();
```

**Don't parse file paths directly:**
```cpp
// WRONG
auto ast = parser.parse("input.hpp", args);

// CORRECT
xccmeta::file f("input.hpp");
auto ast = parser.parse(f.read(), args);
```
