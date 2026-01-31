# xccmeta Documentation

Concise, AI-optimized technical documentation for the xccmeta C++ metadata extraction library.

## Quick Navigation

**Start Here:**
- [Getting Started](getting-started.md) - Build, usage, workflow overview

**Core Modules:**
- [parser](module-parser.md) - Source → AST conversion
- [node](module-node.md) - AST representation and queries
- [type_info](module-type-info.md) - Type introspection
- [tags](module-tags.md) - Metadata annotation system

**Configuration:**
- [compile_args](module-compile-args.md) - Compiler arguments builder

**Utilities:**
- [filter](module-filter.md) - AST node collection with deduplication
- [generator](module-generator.md) - Code generation output writer
- [import](module-import.md) - File I/O and glob patterns
- [source](module-source.md) - Source locations and ranges
- [warnings](module-warnings.md) - Compile-time warning injection

**Optional:**
- [preprocess](module-preprocess.md) - Preprocessor text extraction (rarely needed)
- [base](module-base.md) - Shared library visibility macros

## Architecture Overview

```
Source Code (C/C++)
       ↓
[compile_args] ← Language standard, defines, includes
       ↓
   [parser] ← libclang wrapper, preprocessing
       ↓
    [node] ← AST tree with metadata
       ↓
   [filter] ← Collect nodes by kind/tags
       ↓
 [generator] ← Emit code to files
```

**Data flow:**
1. `importer` reads files (wildcards supported)
2. `parser` converts source to AST (`node` tree)
3. `node::find_descendants()` or `filter` collects relevant declarations
4. Extract metadata via `node` methods (`get_type()`, `get_tags()`, etc.)
5. `generator` writes output with `type_info` data

**Key insight:** All modules operate on `node_ptr`. Once parsed, the AST is self-contained and queryable without re-parsing.

## Typical Workflows

**Enum-to-string generator:**
```cpp
importer → parser → find enums by tag → generator (switch statement)
```

**Serialization codegen:**
```cpp
importer → parser → find structs by tag → iterate fields → generator (binary I/O)
```

**Cross-file type collection:**
```cpp
importer (glob) → parse all → filter (dedupe) → generator (unified registry)
```

## Module Dependencies

```
base (foundation)
 ├─ source
 ├─ compile_args
 ├─ type_info
 └─ tags
      └─ node (depends on all above)
           ├─ parser
           ├─ filter
           ├─ generator
           ├─ import
           ├─ preprocess
           └─ warnings
```

**Include order:** Use `#include <xccmeta.hpp>` for all modules or include specific headers in dependency order.

## Documentation Philosophy

Each module doc follows:
1. **Purpose** - One-sentence problem statement
2. **Why It Exists** - Design rationale, constraints
3. **Core Abstractions** - API surface (no full reference)
4. **When to Use** - Concrete examples
5. **Design Notes** - Performance, safety, trade-offs

**What's NOT here:**
- Full API reference (read headers directly)
- Tutorials (see `examples/` directory)
- Changelog (see git history)
- Internal implementation (see `src/` directory)

## Reading Order

**New users:**
1. [Getting Started](getting-started.md)
2. [parser](module-parser.md)
3. [node](module-node.md)
4. [generator](module-generator.md)

**Advanced topics:**
1. [filter](module-filter.md) - Complex queries
2. [type_info](module-type-info.md) - Type introspection details
3. [tags](module-tags.md) - Annotation patterns
4. [preprocess](module-preprocess.md) - Macro debugging

## Examples Cross-Reference

Docs explain *why* and *when*. Examples show *how*.

**See `examples/` directory:**
- `ast_output.cpp` - Basic parsing and traversal
- `enum_to_string.cpp` - Tag filtering + code generation
- `binary_read_write.cpp` - Type introspection for serialization
- `struct_print.cpp` - Field iteration patterns

**Mapping:**
- Enum example uses: `parser`, `node`, `generator`, `tags`
- Serialization example uses: All of the above + `type_info`
- Import wildcards: `import` module with `importer` class

## Contributing

**Doc updates:** Match existing style—minimal, technical, no filler.

**New modules:** Add `docs/module-<name>.md` following the template structure.

**Keep AI-parseable:** Short paragraphs, bullet lists, code blocks. Avoid prose.
