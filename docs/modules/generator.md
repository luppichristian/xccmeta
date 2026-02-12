---
layout: default
title: Generator
---

# Generator

## Overview

Utility for writing generated code to files with automatic indentation, separators, and compile-time warning emission.

## Why Use This?

Code generators need consistent formatting and diagnostic integration. `generator` provides a simple builder for structured text output without manual indent tracking or file management.

## Types

| Type | Description |
|------|-------------|
| `generator` | Indented output writer targeting a file |

## Functions

### `generator(output_file)`

Construct and open output file for writing. Truncates existing content.

**Signature:** `generator(const std::string& output_file)`

**Parameters:**
- `output_file` — path to the output file

### `out`

Emit a line with current indentation.

**Signature:** `generator& out(const std::string& data)`

**Parameters:**
- `data` — line content (newline appended automatically)

**Returns:** `*this` for chaining

### `indent`

Increase indentation level by one (2 spaces per level).

**Signature:** `generator& indent()`

**Returns:** `*this` for chaining

### `unindent`

Decrease indentation level by one.

**Signature:** `generator& unindent()`

**Returns:** `*this` for chaining

### `separator`

Emit a `//` followed by 77 dashes.

**Signature:** `generator& separator()`

**Returns:** `*this` for chaining

### `named_separator`

Emit a `// === name ===` separator line.

**Signature:** `generator& named_separator(const std::string& name)`

**Parameters:**
- `name` — label for the separator

**Returns:** `*this` for chaining

### `warn`

Add a compile-time warning to the generated output.

**Signatures:**
- `generator& warn(const std::string& message, source_location loc = {})`
- `generator& warn(const std::string& message, const node_ptr node)`

**Parameters:**
- `message` — warning text
- `loc` — optional source location for file:line context
- `node` — node whose location to use

**Returns:** `*this` for chaining

**Notes:** Generates `#pragma message` (MSVC) or `#warning` (GCC/Clang) directives. Warnings appear when the generated file is compiled.

### `done`

Finalize and close the output file. Writes accumulated warnings.

**Signature:** `bool done()`

**Returns:** `true` on success, `false` on file write failure

**Notes:** Also called by the destructor, but prefer explicit `done()` to check the return value.

**Example:**

```cpp
xccmeta::generator gen("output.generated.hpp");

gen.out("#pragma once");
gen.out("#include <string>");
gen.out("");

for (auto& type : types) {
  gen.named_separator(type->get_name());
  gen.out("struct " + type->get_name() + "_Meta {");
  gen.indent();
  gen.out("static constexpr const char* name = \"" + type->get_name() + "\";");
  gen.unindent();
  gen.out("};");
  gen.out("");
}

if (!gen.done()) {
  std::cerr << "Failed to write output\n";
}
```
