# Getting Started

## Purpose

xccmeta is a libclang wrapper for C/C++ metadata extraction. It parses source code into a queryable AST, enabling reflection, code generation, and build pipeline integration.

**Core use case:** Extract type information from tagged C++ declarations and generate boilerplate (serialization, string conversions, introspection).

## Requirements

- **CMake:** 3.19+
- **Compiler:** C++20
- **LLVM/Clang:** 18.x (auto-fetched if missing)

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

**Options:**
- `XCCMETA_BUILD_SHARED=ON` - Build shared library (default: static)
- `XCCMETA_BUILD_TESTS=OFF` - Disable tests
- `XCCMETA_LLVM_FETCH=OFF` - Don't fetch LLVM if missing

## Basic Usage

```cpp
#include <xccmeta.hpp>

// Define source with tagged declarations
const char* src = R"(
  /// @reflect
  struct Vec3 { float x, y, z; };
)";

// Parse
xccmeta::compile_args args;
args.set_standard(xccmeta::language_standard::cxx20);

xccmeta::parser parser;
auto ast = parser.parse(src, args);

// Query
auto structs = ast->find_descendants([](auto& n) {
  return n->get_kind() == xccmeta::node::kind::struct_decl
      && n->has_tag("reflect");
});

// Extract metadata
for (auto& s : structs) {
  for (auto& field : s->get_fields()) {
    auto type = field->get_type().get_spelling();
    auto name = field->get_name();
    // Generate code here
  }
}
```

## Workflow

1. **Tag declarations** - Mark types with `/// @tag_name` or `[[clang::annotate("tag_name")]]`
2. **Parse source** - Produces AST with type info, tags, and source locations
3. **Filter nodes** - Query by kind, tags, or custom predicates
4. **Extract/Generate** - Use metadata to emit code via `generator` class

## Critical Constraints

- **Tag placement:** Comment tags don't work on template args or function params (libclang limitation). Use attributes for those cases.
- **Preprocessing:** Parser handles preprocessing internally via libclang. Don't preprocess input unless you need the expanded text itself.
- **Tag syntax:** Simple literals only: `@tag(arg1, "str", 42)`. No named args or nested expressions.

## Linking

**Static:**
```cmake
target_link_libraries(your_target PRIVATE xccmeta-static)
```

**Shared:**
```cmake
set(XCCMETA_BUILD_SHARED ON)
target_link_libraries(your_target PRIVATE xccmeta)
```
