---
layout: default
title: Warnings
---

# Warnings

## Overview

Generates compiler-specific preprocessor directives that emit warnings when compiled. Integrates code generator diagnostics into the build process.

## Why Use This?

Generated code may have issues detectable only at generation time (incomplete type support, deprecated patterns). `compile_warnings` lets generators inject warnings into output files so users see diagnostics when compiling, not when running the generator.

## Types

| Type | Description |
|------|-------------|
| `compile_warnings` | Warning accumulator that generates preprocessor directives |
| `compile_warnings::entry` | Single warning with message and optional location |

### compile_warnings::entry Fields

| Field | Type | Description |
|-------|------|-------------|
| `message` | `std::string` | Warning text |
| `loc` | `source_location` | Optional file:line context |

## Functions

### `compile_warnings()`

Default constructor.

### `push`

Add a warning.

**Signature:** `compile_warnings& push(const std::string& message, source_location loc = {})`

**Parameters:**
- `message` — warning text
- `loc` — optional source location for context

**Returns:** `*this` for chaining

### `build`

Generate preprocessor directive strings for all accumulated warnings.

**Signature:** `std::string build() const`

**Returns:** string containing `#pragma message` (MSVC) and `#warning` (GCC/Clang) directives

### `get_warnings`

Access the raw warning list.

**Signature:** `const std::vector<entry>& get_warnings() const`

**Notes:**
- Warnings appear when the generated file is compiled, not when the generator runs.
- No deduplication: the same warning can be pushed multiple times.
- Usually accessed through `generator::warn()` rather than used directly.

**Example:**

```cpp
// Standalone usage
xccmeta::compile_warnings warnings;
warnings.push("Raw pointers require manual serialization", field->get_location());
std::string code = warnings.build();

// Through generator (preferred)
xccmeta::generator gen("output.hpp");
gen.warn("Template types not fully supported", node);
```
