---
layout: default
title: Home
---

# xccmeta Documentation

xccmeta is a C++ metadata extraction library built on libclang. It parses C/C++ source code into a queryable AST, enabling reflection, code generation, and build pipeline integration.

## Quick Start

```cpp
#include <xccmeta.hpp>

const char* src = R"(
  /// @reflect
  struct Vec3 { float x, y, z; };
)";

xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
xccmeta::parser parser;
auto ast = parser.parse(src, args);

auto structs = ast->find_descendants([](auto& n) {
  return n->get_kind() == xccmeta::node::kind::struct_decl
      && n->has_tag("reflect");
});

for (auto& s : structs) {
  for (auto& field : s->get_fields()) {
    auto type = field->get_type().get_spelling();
    auto name = field->get_name();
  }
}
```

## Architecture

```
Source Code (C/C++)
       |
[compile_args] <- Language standard, defines, includes
       |
   [parser] <- libclang wrapper, preprocessing
       |
    [node] <- AST tree with metadata
       |
   [filter] <- Collect nodes by kind/tags
       |
 [generator] <- Emit code to files
```

**Data flow:**
1. `importer` reads files (wildcards supported)
2. `parser` converts source to AST (`node` tree)
3. `node::find_descendants()` or `filter` collects relevant declarations
4. Extract metadata via `node` methods (`get_type()`, `get_tags()`, etc.)
5. `generator` writes output with `type_info` data

## Modules

**Core:**
- [parser](modules/parser.md) - Source to AST conversion
- [node](modules/node.md) - AST representation and queries
- [type_info](modules/type-info.md) - Type introspection
- [tags](modules/tags.md) - Metadata annotation system

**Configuration:**
- [compile_args](modules/compile-args.md) - Compiler arguments builder

**Utilities:**
- [filter](modules/filter.md) - AST node collection with deduplication
- [generator](modules/generator.md) - Code generation output writer
- [import](modules/import.md) - File I/O and glob patterns
- [source](modules/source.md) - Source locations and ranges
- [warnings](modules/warnings.md) - Compile-time warning injection

**Optional:**
- [preprocess](modules/preprocess.md) - Preprocessor text extraction (rarely needed)
- [base](modules/base.md) - Shared library visibility macros

## Guides

- [Getting Started](getting-started.md) - Installation, build, and basic usage
- [Architecture](architecture.md) - System design, memory model, and data flow
- [Common Patterns](patterns.md) - Recipes for frequent tasks
- [Examples](examples.md) - Walkthrough of the example programs

## Reading Order

**New users:**
1. [Getting Started](getting-started.md)
2. [parser](modules/parser.md)
3. [node](modules/node.md)
4. [generator](modules/generator.md)

**Advanced topics:**
1. [filter](modules/filter.md) - Complex queries
2. [type_info](modules/type-info.md) - Type introspection details
3. [tags](modules/tags.md) - Annotation patterns
4. [preprocess](modules/preprocess.md) - Macro debugging
