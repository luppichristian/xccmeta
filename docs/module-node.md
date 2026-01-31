# xccmeta_node.hpp

## Purpose

Central AST representation. Each `node` is a parsed C/C++ declaration with kind, type info, source location, tags, and tree structure. All queries and metadata extraction operate on nodes.

## Why It Exists

libclang's AST is cursor-based and ephemeral. `node` caches all relevant data in a persistent, queryable tree that outlives the parse context.

**Design goal:** Provide a high-level, type-safe abstraction over libclang cursors. Enable tree traversal, pattern matching, and filtering without raw libclang API exposure.

## Core Abstractions

**`node`** - AST node (shared_ptr managed: `node_ptr`)

**Identity:**
- `get_kind()` - Enumeration of declaration types (struct, function, field, etc.)
- `get_usr()` - Unique symbol identifier across translation units
- `get_name()` - Simple name ("MyClass")
- `get_qualified_name()` - Fully qualified ("ns::MyClass")

**Type info:**
- `get_type()` - For typed declarations (fields, variables)
- `get_return_type()` - For functions/methods

**Attributes:**
- `get_access()` - public/protected/private
- `is_virtual()`, `is_static()`, `is_const_method()`, etc.
- `is_definition()` - Definition vs. forward declaration

**Tree structure:**
- `get_parent()`, `get_children()`
- `find_child(pred)`, `find_descendants(pred)` - Pattern matching
- `get_bases()`, `get_fields()`, `get_methods()` - Typed queries

**Tags:**
- `has_tag(name)`, `find_tag(name)`, `get_tags()`

**Location:**
- `get_location()`, `get_extent()` - Source position

## Node Kinds

**Types:** `struct_decl`, `class_decl`, `union_decl`, `enum_decl`, `typedef_decl`

**Members:** `field_decl`, `method_decl`, `constructor_decl`, `destructor_decl`

**Functions:** `function_decl`, `parameter_decl`

**Templates:** `class_template`, `function_template`, `template_type_parameter`

**Other:** `namespace_decl`, `base_specifier`, `enum_constant_decl`

See [xccmeta_node.hpp:73-126](../include/xccmeta/xccmeta_node.hpp#L73-L126) for full list.

## When to Use

**Always.** `node` is the primary data structure. All parsed data resides in the tree returned by `parser::parse()`.

**Traversal:**
```cpp
auto structs = ast->find_descendants([](auto& n) {
  return n->is_record_decl() && n->has_tag("reflect");
});
```

**Member iteration:**
```cpp
for (auto& field : struct_node->get_fields()) {
  if (field->get_access() == access_specifier::private_) continue;
  auto type = field->get_type().get_spelling();
  auto name = field->get_name();
}
```

**Method queries:**
```cpp
auto ctors = class_node->find_children([](auto& n) {
  return n->get_kind() == node::kind::constructor_decl && !n->is_deleted();
});
```

**Enum handling:**
```cpp
if (node->get_kind() == node::kind::enum_decl) {
  for (auto& constant : node->get_enum_constants()) {
    auto value = constant->get_enum_value();
    auto name = constant->get_name();
  }
}
```

## Design Notes

**Shared ownership:** `node_ptr` is `std::shared_ptr<node>`. Parent uses weak_ptr to avoid cycles. Tree lifetime managed by root node.

**Immutability (public):** All setters are protected. Only `parser` can modify nodes. User code queries read-only data.

**Tree consistency:** `add_child()` updates both parent and child pointers atomically. Manual tree modification unsupported.

**Predicate efficiency:** `find_descendants()` is depth-first search. For large trees (>10k nodes), consider caching results or using `filter` class for complex criteria.

**Template support:** Template declarations exist as nodes, but instantiations are not traversed. Only explicit specializations appear in AST.

**USR stability:** USR (Unified Symbol Resolution) strings are deterministic across parses of the same code. Use for cross-file entity matching.

## Architecture Note

**Why parent is weak_ptr:** Prevents reference cycles. Dropping root node cleans entire tree. Accessing parent requires `lock()` check (safe pattern: `if (auto p = node->get_parent())`).

**Why shared_ptr, not unique_ptr:** Nodes appear in multiple query results. Shared ownership prevents dangling references when results outlive the traversal.
