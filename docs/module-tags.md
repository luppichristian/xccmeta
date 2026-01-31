# xccmeta_tags.hpp

## Purpose

Extracts and represents metadata annotations from source comments or clang attributes. Enables tag-based filtering and parameterized code generation.

## Why It Exists

Reflection requires marking declarations for processing. C++ lacks native reflection, so xccmeta extracts custom annotations from two sources:
1. **Comment tags:** `/// @tag_name(args)`
2. **Attribute tags:** `[[clang::annotate("tag_name")]]`

**Design choice:** Comment tags are more readable. Attributes work everywhere libclang supports them (including template args, where comment tags fail).

## Core Abstractions

**`tag`** - Parsed annotation with name and arguments
- `get_name()` - Full tag identifier (e.g., `"reflect"`)
- `get_args()` - Vector of string arguments
- `get_args_combined()` - Comma-separated args as single string
- `get_full()` - "tag_name(arg1, arg2)"

**`tag::parse(str)`** - Static parser for tag strings

## Tag Syntax

**Comment style:**
```cpp
/// @reflect
/// @serialize(binary, version=2)  // version=2 parsed as single arg
struct Data {};

struct Other {}; ///< @skip  // Trailing comment
```

**Attribute style:**
```cpp
struct [[clang::annotate("reflect")]] Data {};

template<typename [[clang::annotate("numeric")]] T>
void process(T value);
```

**Limitations:**
- Comment tags **fail** on template parameters, function parameters (libclang doesn't associate comments)
- Use attributes for these cases
- Args are simple literals only: no nested parens, no named args, no expressions

## When to Use

**Access via `node`:**
```cpp
if (node->has_tag("reflect")) {
  auto tag = node->find_tag("serialize");
  if (tag) {
    auto& args = tag->get_args();
    // args[0] == "binary", args[1] == "version=2"
  }
}
```

**Multi-tag filtering:**
```cpp
auto nodes = ast->find_descendants([](auto& n) {
  return n->has_tags({"reflect", "introspect"});
});
```

**Tag-based code generation:**
```cpp
for (auto& field : struct_node->get_fields()) {
  if (field->has_tag("no_serialize")) continue;

  auto default_tag = field->find_tag("default");
  auto default_val = default_tag ? default_tag->get_args()[0] : "{}";
}
```

## Design Notes

**Parsing rules:**
- Tag format: `@name(arg1, arg2)` or just `@name`
- Args split on commas (no escape mechanism)
- Whitespace trimmed
- String quotes preserved in arg values

**Parent tag inheritance:** Use `node::get_parent_tags()` to walk up the tree. Useful for namespace-level tags affecting all children.

**Tag storage:** Tags are vectors on `node`. Duplicate tags allowed. Use `find_tag()` for first match or `find_tags()` for all.

**Performance:** Tag extraction happens during parsing. Queries are O(n) over tag vector (typically small, <10 tags per node).

**Portability:** Comment tags are pure C++. Attribute tags require clang-compatible compiler for the *target code* (not the tool build).
