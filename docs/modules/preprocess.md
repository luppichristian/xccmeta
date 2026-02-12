---
layout: default
title: Preprocess
---

# Preprocess

## Overview

Optional module. Exposes preprocessed C/C++ source text (macro-expanded, conditionals evaluated, includes resolved). The parser already preprocesses internally — this module is only for accessing the expanded text itself.

## Why Use This?

Most workflows don't need this module. Use it when:
- Debugging macro expansions
- Emitting `#line` directives mapped to original source
- Displaying preprocessed code to users

The `parser` class handles preprocessing automatically. Do **not** preprocess input before calling `parser::parse()`.

## Types

| Type | Description |
|------|-------------|
| `preprocessor_context` | Stateful preprocessor cache (movable, non-copyable) |
| `preprocessor` | File-based preprocessor with context sharing |

## Functions

### preprocessor_context

#### `preprocessor_context()`

Default constructor.

#### `preprocessor_context(input, args)`

Construct with initial source and compile arguments.

**Signature:** `preprocessor_context(const std::string& input, const compile_args& args = compile_args())`

**Parameters:**
- `input` — source text defining macros/includes
- `args` — compiler arguments

#### `apply`

Preprocess source text using this context.

**Signature:** `std::string apply(const std::string& to_preprocess, const compile_args& args = compile_args()) const`

**Parameters:**
- `to_preprocess` — source text to preprocess
- `args` — compiler arguments

**Returns:** macro-expanded source text

### preprocessor

#### `preprocessor(file, args)`

Construct from a single file.

**Signature:** `explicit preprocessor(const file& file, const compile_args& args = compile_args())`

#### `preprocessor(files, args)`

Construct from multiple files.

**Signature:** `explicit preprocessor(const std::vector<file>& files, const compile_args& args = compile_args())`

#### `get_preprocessed_content`

Get the expanded source text for each input file.

**Signature:** `const std::vector<std::string>& get_preprocessed_content() const`

**Returns:** vector of preprocessed strings (one per input file)

#### `get_context`

Get the reusable preprocessing context.

**Signature:** `const preprocessor_context& get_context() const`

**Notes:**
- Preprocessing is as expensive as parsing (~1MB/s). Avoid unless necessary.
- Output is plain text with no AST structure.
- Context doesn't track include guards; re-including the same header may cause redefinition warnings.

**Example:**

```cpp
// Inspect macro expansion
xccmeta::file f("input.hpp");
xccmeta::preprocessor pp(f, args);
auto expanded = pp.get_preprocessed_content()[0];
std::cout << expanded;

// Share context across files
xccmeta::preprocessor_context ctx("#define PLATFORM_LINUX", args);
auto result1 = ctx.apply(file1.read(), args);
auto result2 = ctx.apply(file2.read(), args);
```
