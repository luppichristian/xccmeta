---
layout: default
title: Compile Args
---

# Compile Args

## Overview

Manages compiler arguments passed to libclang for parsing. Controls language standard, preprocessor defines, include paths, and target configuration.

## Why Use This?

libclang requires explicit compiler flags to parse code correctly. This class provides a fluent builder API instead of raw argument string construction, and ensures compatibility across platforms and compilers.

## Types

| Type | Description |
|------|-------------|
| `compile_args` | Fluent builder for compiler arguments |
| `language_standard` | Enum of supported C/C++ standards (c89 through cxx26) |
| `language_mode` | Enum of language modes (c, cxx, objective_c, objective_cxx) |

## Functions

### `compile_args()`

Default constructor. Creates empty argument list.

### `add`

Add a raw compiler argument.

**Signature:** `void add(const std::string& arg)`

**Parameters:**
- `arg` — raw compiler flag (e.g., `-Wall`)

### `add_many`

Add multiple raw compiler arguments.

**Signature:** `void add_many(const std::vector<std::string>& args_to_add)`

**Parameters:**
- `args_to_add` — list of raw compiler flags

### `clear`

Remove all arguments.

**Signature:** `void clear()`

### `get_args`

Get the current argument list.

**Signature:** `const std::vector<std::string>& get_args() const`

**Returns:** reference to the internal argument vector

### `set_standard`

Set the language standard.

**Signature:** `compile_args& set_standard(language_standard std)`

**Parameters:**
- `std` — language standard enum value (e.g., `language_standard::cxx20`)

**Returns:** `*this` for chaining

### `set_language`

Set the language mode.

**Signature:** `compile_args& set_language(language_mode lang)`

**Parameters:**
- `lang` — language mode (e.g., `language_mode::cxx`)

**Returns:** `*this` for chaining

### `add_include_path`

Add a single include path (`-I<path>`).

**Signature:** `compile_args& add_include_path(const std::string& path)`

**Parameters:**
- `path` — include directory path

**Returns:** `*this` for chaining

### `add_include_paths`

Add multiple include paths.

**Signature:** `compile_args& add_include_paths(const std::vector<std::string>& paths)`

**Parameters:**
- `paths` — list of include directory paths

**Returns:** `*this` for chaining

### `define`

Define a preprocessor macro.

**Signatures:**
- `compile_args& define(const std::string& name)` — define flag (`-D<name>`)
- `compile_args& define(const std::string& name, const std::string& value)` — define with string value
- `compile_args& define(const std::string& name, int value)` — define with integer value

**Parameters:**
- `name` — macro name
- `value` — macro value (optional)

**Returns:** `*this` for chaining

### `undefine`

Undefine a preprocessor macro (`-U<name>`).

**Signature:** `compile_args& undefine(const std::string& name)`

**Parameters:**
- `name` — macro name to undefine

**Returns:** `*this` for chaining

### `set_target`

Set target triple for cross-compilation (`--target=<triple>`).

**Signature:** `compile_args& set_target(const std::string& triple)`

**Parameters:**
- `triple` — target triple (e.g., `"x86_64-linux-gnu"`, `"aarch64-linux-gnu"`)

**Returns:** `*this` for chaining

**Notes:** Affects `sizeof`, predefined macros, and type alignment.

### `set_pointer_size`

Set pointer size for cross-compilation (`-m32` or `-m64`).

**Signature:** `compile_args& set_pointer_size(int bits)`

**Parameters:**
- `bits` — pointer size in bits (32 or 64)

**Returns:** `*this` for chaining

### `modern_cxx` (static)

Create a preset configured for modern C++ development.

**Signature:** `static compile_args modern_cxx(language_standard std = language_standard::cxx20)`

**Parameters:**
- `std` — C++ standard to use (default: C++20)

**Returns:** configured `compile_args` instance

### `modern_c` (static)

Create a preset configured for C development.

**Signature:** `static compile_args modern_c(language_standard std = language_standard::c17)`

**Parameters:**
- `std` — C standard to use (default: C17)

**Returns:** configured `compile_args` instance

### `minimal` (static)

Create a preset with no default flags.

**Signature:** `static compile_args minimal()`

**Returns:** empty `compile_args` instance

**Example:**

```cpp
auto args = xccmeta::compile_args::modern_cxx();
args.define("DEBUG", "1")
    .add_include_path("/opt/custom/include")
    .set_target("aarch64-linux-gnu");
```
