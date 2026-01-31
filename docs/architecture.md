# Architecture

## System Design

xccmeta is a **libclang wrapper** optimized for metadata extraction and code generation pipelines. It trades generality for usability in the reflection/codegen domain.

## Core Principles

**1. Parse once, query many times**
- AST persists as `shared_ptr<node>` tree
- No re-parsing for different queries
- Parser is stateless; AST is self-contained

**2. Tag-driven workflows**
- User annotates code with `/// @tag_name`
- Parser extracts tags into AST
- Filters/generators select nodes by tags
- Enables opt-in reflection without language changes

**3. Libclang isolation**
- Raw libclang types hidden behind `node`, `type_info`, `source_location`
- Users never touch `CXCursor`, `CXType`, etc.
- Enables implementation changes without API breaks

**4. Build pipeline integration**
- Designed for CMake `add_custom_command` integration
- File I/O via `importer`, output via `generator`
- Warnings injected into generated code (compile-time diagnostics)

## Architecture Layers

```
┌─────────────────────────────────────────┐
│  User Code (code generators, tools)    │
├─────────────────────────────────────────┤
│  High-Level API                         │
│  - filter (deduplication, bulk ops)     │
│  - generator (formatted output)         │
│  - importer (file I/O + globs)          │
├─────────────────────────────────────────┤
│  Core AST API                           │
│  - node (tree structure, queries)       │
│  - type_info (type introspection)       │
│  - tags (metadata extraction)           │
│  - source (location tracking)           │
├─────────────────────────────────────────┤
│  Parser Layer                           │
│  - parser (libclang bridge)             │
│  - compile_args (flag management)       │
│  - [preprocessor (optional text dump)]  │
├─────────────────────────────────────────┤
│  libclang (LLVM 18.x)                   │
└─────────────────────────────────────────┘
```

## Data Flow

**Typical generator pipeline:**

```
1. INPUT: C++ source files
   ↓
2. importer::get_files() → vector<file>
   ↓
3. for each file:
     file.read() → string
     ↓
4. parser.parse(source, args) → node_ptr (AST root)
   ↓
5. node::find_descendants(predicate) → vector<node_ptr>
   ↓
6. filter.add(nodes) + filter.clean() → deduplicated types
   ↓
7. for each type:
     Extract: type_info, tags, fields, methods
     ↓
8. generator.out(generated_code) → writes to file
   ↓
9. OUTPUT: Generated .hpp file
```

## Memory Model

**Ownership:**
- AST nodes: `shared_ptr<node>` (root owns tree)
- Parent links: `weak_ptr<node>` (break cycles)
- Filter collections: `shared_ptr` copies (no ownership transfer)

**Lifetime:**
- Parser creates AST, returns root
- Dropping root destroys entire tree (cascade via `shared_ptr`)
- Query results (`vector<node_ptr>`) extend node lifetime

**Why shared_ptr:**
- Nodes appear in multiple query results
- Prevents dangling references when results outlive traversal
- Automatic cleanup when all references dropped

**Cost:**
- 10k-line file → ~50k nodes → ~3MB (rough estimate)
- Large codebases: parse incrementally, don't hold all ASTs

## Preprocessing Model

**Two-phase approach:**

1. **Parser's internal preprocessing** (always happens)
   - libclang evaluates `#define`, `#ifdef`, `#include`
   - AST reflects post-preprocessor view
   - User doesn't see expanded text

2. **Optional text preprocessing** (`preprocessor` module)
   - Exposes macro-expanded source as string
   - Rarely needed (debugging, `#line` directives)
   - Separate from parsing (doesn't affect AST)

**Design rationale:** Most users want AST metadata, not preprocessed text. Parser handles preprocessing transparently.

## Extensibility Points

**Adding new node kinds:**
1. Update `node::kind` enum ([xccmeta_node.hpp:73](../include/xccmeta/xccmeta_node.hpp#L73))
2. Map libclang cursor kind in `parser_impl` (internal)
3. Add convenience methods to `node` class if needed

**Adding type properties:**
1. Add member to `type_info` class ([xccmeta_type_info.hpp:34](../include/xccmeta/xccmeta_type_info.hpp#L34))
2. Update `parser_impl` to populate during AST construction
3. Add getter method

**Custom tag parsing:**
- Tags are strings; parse `tag::get_args()` in generator
- No built-in schema validation (intentional flexibility)

## Performance Characteristics

**Parsing:** O(n) in source size, ~1MB/s (varies by include depth)
- Bottleneck: libclang preprocessing + AST construction
- Mitigation: Cache parse results, incremental builds

**Traversal:** O(n) in node count
- `find_descendants()` is depth-first search
- 50k nodes → ~10ms on modern CPU
- Mitigation: Cache query results, use `filter` for deduplication

**Type queries:** O(1)
- All type info precomputed during parsing
- `get_type().is_pointer()` is member access, not recomputation

**Filter deduplication:** O(n²) in worst case
- Uses linear search over USR strings
- Bottleneck for >1000 unique types
- Mitigation: Use `std::unordered_set<std::string>` externally if needed

## Error Handling Strategy

**Parse failures:** Return `nullptr`
- No exceptions thrown
- Diagnostics written to stderr (libclang behavior)
- User checks `if (!ast)` after parse

**File I/O:** Exceptions
- `file::read()` throws `std::runtime_error` if missing
- `file::write()` returns `bool` (silent failure)

**Invalid queries:** Undefined behavior
- Accessing `get_parent()` on root → `nullptr` (safe)
- Accessing fields on non-record node → empty vector (safe)
- No runtime validation of node kind before queries

**Design rationale:** Parsing is the only operation that commonly fails. File I/O errors are exceptional. Node queries assume correct usage (checked by user via `get_kind()`).

## Build System Integration

**CMake pattern:**
```cmake
add_custom_command(
  OUTPUT generated.hpp
  COMMAND my_generator ${CMAKE_SOURCE_DIR}/input.hpp
  DEPENDS input.hpp my_generator
)
add_library(mylib generated.hpp ...)
```

**Generator tool:**
- Links against `xccmeta-static`
- Reads source via `importer`
- Writes output via `generator`
- Runs at build time (not install time)

**Workflow:** Edit source → CMake re-runs generator → Compiler sees updated output

## Thread Safety

**Not thread-safe:**
- `parser` instances (use one per thread)
- `generator` instances (file I/O not synchronized)

**Thread-safe after parsing:**
- Read-only queries on `node` tree (immutable after construction)
- Multiple threads can traverse same AST concurrently

**Recommendation:** Parse in parallel (separate `parser` per thread), merge results in main thread.

## Versioning Strategy

**API stability:** Unstable (pre-1.0)
- Breaking changes possible in minor versions
- Semantic versioning planned for 1.0 release

**ABI stability:** None
- Recompile tools when upgrading library
- No binary compatibility guarantees between versions

**Compatibility:** libclang 18.x
- Upgrade requires testing (libclang API changes)
- Auto-fetch mechanism pins version (llvmorg-18.1.8)

## Design Trade-offs

**Simplicity over generality:**
- No support for full compile pipeline (just parsing)
- No cross-reference resolution (USR strings only)
- No incremental parsing (always full parse)

**Usability over performance:**
- `shared_ptr` over raw pointers (safety)
- String copies over views (lifetime clarity)
- Eager type info extraction (no lazy evaluation)

**Reflection-focused:**
- Tag system assumes annotation-driven workflows
- Type introspection optimized for codegen, not analysis
- No support for statement-level AST (declarations only)

**Result:** Fast development of reflection-based tools. Not suitable for compiler-quality analysis or refactoring tools.
