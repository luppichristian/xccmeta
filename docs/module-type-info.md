# xccmeta_type_info.hpp

## Purpose

Encapsulates resolved type information extracted from AST nodes. Provides qualifiers, type categories, pointer/reference indirection, array dimensions, and size/alignment data.

## Why It Exists

libclang's type API is verbose and cursor-dependent. This class caches type properties during parsing for efficient query access without re-traversing the AST.

**Design constraint:** Type resolution happens at parse time. Changing compile args afterward doesn't update existing `type_info` instances.

## Core Abstractions

**`type_info`** - Immutable type descriptor (constructed by parser only)

**Type names:**
- `get_spelling()` - As written in source (`const int*`)
- `get_canonical()` - Fully resolved (`int const *`)
- `get_unqualified_spelling()` - Stripped qualifiers (`int*`)

**Qualifiers:** `is_const()`, `is_volatile()`, `is_restrict()`

**Categories:** `is_pointer()`, `is_reference()`, `is_array()`, `is_function_pointer()`

**Indirection:**
- `get_pointee_type()` - For pointers/references
- `get_array_element_type()`, `get_array_size()` - For arrays

**Size info:**
- `get_size_bytes()` - Returns `-1` if unavailable (incomplete types)
- `get_alignment()` - Returns `-1` if unavailable

**Type classification:** `is_integral()`, `is_floating_point()`, `is_signed()`, `is_builtin()`

## When to Use

**Access via `node`:**
```cpp
auto field = /* node representing a field */;
auto type = field->get_type();

if (type.is_pointer() && type.get_pointee_type() == "char") {
  // C-string
}
```

**Common pattern - array field handling:**
```cpp
if (type.is_array()) {
  auto elem = type.get_array_element_type();
  auto count = type.get_array_size(); // -1 if unsized
}
```

**Size-dependent code generation:**
```cpp
if (type.get_size_bytes() != -1) {
  gen.out("static_assert(sizeof(T) == " + std::to_string(type.get_size_bytes()) + ");");
}
```

## Design Notes

**Immutability:** No public setters. Instances are populated by parser during AST construction.

**Canonical vs. Spelling:** `get_canonical()` resolves typedefs. Use for type comparison. Use `get_spelling()` for code generation to preserve source intent.

**Size availability:** Depends on compile args (target triple, pointer width). Incomplete types (forward declarations, templates without instantiation) return `-1`.

**Performance:** Type info is computed once during parsing. Queries are O(1) member access.
