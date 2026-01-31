# xccmeta_parser.hpp

## Purpose

Converts C/C++ source text into a queryable AST. Entry point for all metadata extraction workflows.

## Why It Exists

libclang API is low-level and stateful. `parser` encapsulates index creation, translation unit management, and AST traversal into a single-call interface.

**Design goal:** Parse once, query many times. AST persists independently of parser instance.

## Core Abstractions

**`parser`** - Stateless parser (movable, non-copyable)

**`parse(input, args)`** - Main entry point
- `input` - Source code string (not a file path)
- `args` - Compiler arguments
- Returns: `node_ptr` to translation unit root (or nullptr on failure)

**`merge(a, b, args)`** - Combine two ASTs
- Merges children of two translation units
- Useful for multi-file processing
- Returns: New root node containing all children

## When to Use

**Every workflow starts here:**
```cpp
xccmeta::parser parser;
auto ast = parser.parse(source, args);
if (!ast) {
  // Parse failed - check stderr for diagnostics
}
```

**Multi-file merge:**
```cpp
auto ast1 = parser.parse(file1_content, args);
auto ast2 = parser.parse(file2_content, args);
auto merged = parser.merge(ast1, ast2, args);
```

**In-memory parsing:**
```cpp
std::string generated = generate_code();
auto ast = parser.parse(generated, args);
```

## Preprocessing Behavior

**Critical: Parser preprocesses input automatically.**

libclang expands macros, evaluates conditionals, and processes includes *before* AST construction. This means:

- `#define FOO` in source is expanded
- `args.define("BAR")` affects parsing
- `#include` directives are followed (if files exist)
- The AST contains the *post-preprocessor* view

**Do NOT preprocess input yourself** unless you need the expanded text for non-parsing purposes.

## Design Notes

**Stateless:** Each `parse()` call is independent. No reusable parse context across calls.

**Error handling:** Parse failures return `nullptr`. libclang writes diagnostics to stderr (not capturable currently).

**Performance:** libclang parses at ~1MB/s (highly dependent on include depth). Expect multi-second parse times for large headers.

**Memory:** AST nodes are heap-allocated via `shared_ptr`. A 10k-line file may produce 50k+ nodes. Profile before parsing entire codebases.

**Thread safety:** `parser` is not thread-safe. Use separate instances per thread.

## Merge Semantics

**What merge does:**
- Creates new root `translation_unit` node
- Copies children from both input ASTs
- Does NOT deduplicate identical symbols
- Does NOT resolve cross-file references

**Use case:** Collecting declarations from multiple headers for bulk code generation.

**Limitation:** Merged AST has no inter-file semantic links. USRs remain valid for cross-referencing, but parent-child relationships don't span original files.

## Parse Input Format

**Not a file path.** Input is raw source text:
```cpp
// WRONG
auto ast = parser.parse("my_file.hpp", args);

// CORRECT
auto content = read_file("my_file.hpp");
auto ast = parser.parse(content, args);
```

**Includes:** Relative `#include` paths resolve from current working directory, not input pseudo-filename.

**Override:** Use `args.add_include_path()` to control include resolution.

## Comparison with Preprocessor Module

**Parser (this module):** AST construction. Preprocessing is internal and opaque.

**Preprocessor module:** Exposes preprocessed *text*. Use when you need expanded source for display/debugging, not for parsing.

**Typical workflow:** Parse directly. Ignore preprocessor module unless emitting `#line` directives or debugging macro expansions.
