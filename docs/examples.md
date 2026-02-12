---
layout: default
title: Examples
---

# Examples

Complete working examples are in the [examples/](https://github.com/luppichristian/xccmeta/tree/main/examples) directory. Each demonstrates a different xccmeta workflow.

## ast_output

**File:** `examples/ast_output.cpp`

Parses C++ source containing structs tagged with `@reflect` and prints detailed AST information (kind, name, type, access, tags) for each struct and its members.

**Modules used:** `parser`, `node`, `compile_args`, `tags`

**What it demonstrates:**
- Parsing source code with `parser::parse()`
- Finding tagged nodes with `find_descendants()`
- Iterating fields, methods, and base classes
- Querying node properties (`get_kind()`, `get_type()`, `get_access()`)

## enum_to_string

**File:** `examples/enum_to_string.cpp`

Generates `to_string()` and `from_string()` conversion functions for enums tagged with `@reflect`. Output is written to `enum_strings.generated.hpp`.

**Modules used:** `parser`, `node`, `generator`, `tags`, `compile_args`

**What it demonstrates:**
- Tag-based enum filtering
- Code generation with `generator` class
- Iterating enum constants with `get_enum_constants()`
- Using `get_qualified_name()` for fully qualified output

## struct_print

**File:** `examples/struct_print.cpp`

Generates `print()` functions for structs tagged with `@reflect` that output field names and values. Output is written to `struct_print.generated.hpp`.

**Modules used:** `parser`, `node`, `generator`, `tags`, `type_info`, `compile_args`

**What it demonstrates:**
- Field iteration with `get_fields()`
- Type property queries (`is_pointer()`, `is_array()`, `is_builtin()`)
- Generating formatted C++ output with indentation

## binary_read_write

**File:** `examples/binary_read_write.cpp`

The most comprehensive example. Generates endian-agnostic binary `read()` and `write()` serialization functions for structs tagged with `@reflect`. Output is written to `binary_serialization.generated.hpp`.

**Modules used:** `parser`, `node`, `generator`, `tags`, `type_info`, `compile_args`

**What it demonstrates:**
- Full type introspection for serialization decisions
- Handling arrays, pointers, nested types, and builtin types
- Using `get_size_bytes()` and `get_alignment()` for binary layout
- Generating multi-function output (read + write per struct)

## Module Coverage

| Example | parser | node | generator | tags | type_info | filter | import |
|---------|--------|------|-----------|------|-----------|--------|--------|
| ast_output | x | x | | x | | | |
| enum_to_string | x | x | x | x | | | |
| struct_print | x | x | x | x | x | | |
| binary_read_write | x | x | x | x | x | | |
