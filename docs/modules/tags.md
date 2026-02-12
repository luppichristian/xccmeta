---
layout: default
title: Tags
---

# Tags

## Overview

Extracts and represents metadata annotations from source comments or clang attributes. Enables tag-based filtering and parameterized code generation.

## Why Use This?

C++ lacks native reflection. xccmeta extracts custom annotations from two sources to enable opt-in reflection:
1. **Comment tags:** `/// @tag_name(args)` — more readable
2. **Attribute tags:** `[[clang::annotate("tag_name")]]` — works everywhere, including template/function parameters

## Types

| Type | Description |
|------|-------------|
| `tag` | Parsed annotation with a name and arguments |

## Functions

### `tag()`

Default constructor.

### `tag(name, args)`

Construct a tag with a name and argument list.

**Signature:** `tag(const std::string& name, const std::vector<std::string>& args)`

**Parameters:**
- `name` — tag identifier
- `args` — list of string arguments

### `parse` (static)

Parse a tag from a string.

**Signature:** `static tag parse(const std::string& to_parse)`

**Parameters:**
- `to_parse` — string in format `"tag_name(arg1, arg2)"`

**Returns:** parsed `tag` instance

### `get_name`

Get the tag name.

**Signature:** `const std::string& get_name() const`

**Returns:** tag identifier (e.g., `"reflect"`)

### `get_args`

Get the argument list.

**Signature:** `const std::vector<std::string>& get_args() const`

**Returns:** vector of string arguments

### `get_args_combined`

Get arguments as a single comma-separated string.

**Signature:** `std::string get_args_combined() const`

**Returns:** combined args (e.g., `"arg1, arg2"`)

### `get_full`

Get the full tag representation.

**Signature:** `std::string get_full() const`

**Returns:** full string (e.g., `"tag_name(arg1, arg2)"`)

**Notes:**
- Comment tags (`/// @tag`) don't work on template parameters or function parameters (libclang limitation). Use `[[clang::annotate("tag")]]` for those cases.
- Arguments are simple literals only: no nested parentheses, no named arguments.
- Tags are typically accessed through `node` methods: `has_tag()`, `find_tag()`, `get_tags()`.

**Example:**

```cpp
// Comment style
/// @reflect
/// @serialize(binary, version=2)
struct Data {};

// Attribute style
template<typename [[clang::annotate("numeric")]] T>
void process(T value);

// Querying tags
if (node->has_tag("reflect")) {
  auto tag = node->find_tag("serialize");
  if (tag) {
    auto& args = tag->get_args(); // {"binary", "version=2"}
  }
}
```
