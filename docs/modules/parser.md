---
layout: default
title: Parser
---

# Parser

## Overview

Converts C/C++ source text into a queryable AST. Entry point for all metadata extraction workflows.

## Why Use This?

libclang's API is low-level and stateful. `parser` encapsulates index creation, translation unit management, and AST traversal into a single-call interface. Parse once, then query the resulting `node` tree as many times as needed.

## Types

| Type | Description |
|------|-------------|
| `parser` | Stateless parser (movable, non-copyable) |

## Functions

### `parser()`

Default constructor.

### `parse`

Parse source code into an AST.

**Signature:** `std::shared_ptr<node> parse(const std::string& input, const compile_args& args)`

**Parameters:**
- `input` — source code text (not a file path)
- `args` — compiler arguments controlling language standard, defines, includes

**Returns:** `node_ptr` to the translation unit root, or `nullptr` on parse failure

**Notes:**
- Input must be source text, not a file path. Read files first: `parser.parse(file.read(), args)`.
- The parser preprocesses input automatically via libclang (macros, `#ifdef`, `#include`). Do not preprocess beforehand.
- Parse failures write diagnostics to stderr. Check for `nullptr` return.
- Parser instances are not thread-safe. Use separate instances per thread.

**Example:**

```cpp
xccmeta::parser parser;
xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

auto ast = parser.parse(source_code, args);
if (!ast) {
  std::cerr << "Parse failed\n";
  return 1;
}
```

### `merge`

Merge two ASTs into a single tree.

**Signature:** `std::shared_ptr<node> merge(std::shared_ptr<node> a, std::shared_ptr<node> b, const compile_args& args)`

**Parameters:**
- `a` — first AST root
- `b` — second AST root
- `args` — compiler arguments

**Returns:** new root `node_ptr` containing children from both ASTs

**Notes:**
- Does not deduplicate identical symbols. Use `filter` for deduplication.
- Does not resolve cross-file references. USR strings remain valid for cross-referencing.
- Parent-child relationships don't span original files.

**Example:**

```cpp
auto ast1 = parser.parse(file1.read(), args);
auto ast2 = parser.parse(file2.read(), args);
auto merged = parser.merge(ast1, ast2, args);
```
