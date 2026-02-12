---
layout: default
title: Type Info
---

# Type Info

## Overview

Encapsulates resolved type information extracted from AST nodes. Provides qualifiers, type categories, pointer/reference indirection, array dimensions, and size/alignment data.

## Why Use This?

Type information is essential for code generation decisions (serialization, formatting, validation). `type_info` caches all type properties during parsing for efficient O(1) query access.

## Types

| Type | Description |
|------|-------------|
| `type_info` | Immutable type descriptor (constructed by parser only) |

## Functions

### `get_spelling`

Get the type name as written in source.

**Signature:** `const std::string& get_spelling() const`

**Returns:** source-level type string (e.g., `"const int*"`)

### `get_canonical`

Get the fully resolved type name (resolves typedefs).

**Signature:** `const std::string& get_canonical() const`

**Returns:** canonical type string (e.g., `"int const *"`)

**Notes:** Use for type comparison. Use `get_spelling()` for code generation to preserve source intent.

### `get_unqualified_spelling`

Get the type name with qualifiers removed.

**Signature:** `std::string get_unqualified_spelling() const`

**Returns:** type string without const/volatile/restrict

### `is_const`

**Signature:** `bool is_const() const`

### `is_volatile`

**Signature:** `bool is_volatile() const`

### `is_restrict`

**Signature:** `bool is_restrict() const`

### `is_pointer`

**Signature:** `bool is_pointer() const`

### `is_reference`

**Signature:** `bool is_reference() const`

### `is_lvalue_reference`

**Signature:** `bool is_lvalue_reference() const`

### `is_rvalue_reference`

**Signature:** `bool is_rvalue_reference() const`

### `is_array`

**Signature:** `bool is_array() const`

### `is_function_pointer`

**Signature:** `bool is_function_pointer() const`

### `get_pointee_type`

Get the type being pointed to (for pointers/references).

**Signature:** `const std::string& get_pointee_type() const`

### `get_array_element_type`

Get the element type of an array.

**Signature:** `const std::string& get_array_element_type() const`

### `get_array_size`

Get the array size.

**Signature:** `std::int64_t get_array_size() const`

**Returns:** array element count, or `-1` for unsized arrays

### `get_size_bytes`

Get the type size in bytes.

**Signature:** `std::int64_t get_size_bytes() const`

**Returns:** size in bytes, or `-1` if unavailable (incomplete types, forward declarations)

**Notes:** Depends on compile args (target triple, pointer width).

### `get_alignment`

Get the type alignment in bytes.

**Signature:** `std::int64_t get_alignment() const`

**Returns:** alignment in bytes, or `-1` if unavailable

### `is_valid`

**Signature:** `bool is_valid() const`

### `is_void`

**Signature:** `bool is_void() const`

### `is_integral`

**Signature:** `bool is_integral() const`

### `is_floating_point`

**Signature:** `bool is_floating_point() const`

### `is_arithmetic`

**Signature:** `bool is_arithmetic() const`

### `is_signed`

**Signature:** `bool is_signed() const`

### `is_unsigned`

**Signature:** `bool is_unsigned() const`

### `is_builtin`

**Signature:** `bool is_builtin() const`

### `has_qualifiers`

**Signature:** `bool has_qualifiers() const`

### `to_string`

Debug string representation.

**Signature:** `std::string to_string() const`

**Example:**

```cpp
auto type = field->get_type();

if (type.is_array()) {
  auto elem = type.get_array_element_type();
  auto count = type.get_array_size(); // -1 if unsized
} else if (type.is_pointer()) {
  auto pointee = type.get_pointee_type();
} else if (type.is_builtin()) {
  auto size = type.get_size_bytes(); // -1 if unavailable
}
```
