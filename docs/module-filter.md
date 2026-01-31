# xccmeta_filter.hpp

## Purpose

Collects unique AST nodes matching configurable criteria (kinds, tags, parent/child inclusion). Deduplicates by USR and provides bulk filtering operations.

## Why It Exists

Ad-hoc `find_descendants()` works for simple queries. Complex filters (multi-tag logic, kind whitelists, parent propagation) require boilerplate. `filter` encapsulates common patterns.

**Design goal:** Reusable filtering config for consistent node collection across multiple ASTs or traversals.

## Core Abstractions

**`filter`** - Container of unique `node_ptr` (deduped by USR)

**`filter::config`** - Filtering rules
- `allowed_kinds` - Whitelist of `node::kind` (empty = all kinds)
- `grab_tag_names` - Include nodes with any of these tags
- `avoid_tag_names` - Exclude nodes with any of these tags
- `child_node_inclusion` - How to handle children of matched nodes
- `parent_node_inclusion` - How to handle parents of matched nodes

**Inclusion modes:**
- `exclude` - Only the matched node
- `include` - Matched node + direct children
- `include_recursively` - Matched node + all descendants

## When to Use

**Use when:**
- Collecting nodes across multiple files/ASTs
- Need deduplication (same symbol parsed multiple times)
- Complex tag/kind logic applied uniformly

**Don't use when:**
- Simple one-time query (`find_descendants` is clearer)
- Need duplicate nodes (e.g., all occurrences of a name)

## Example

**Basic filtering:**
```cpp
filter::config cfg;
cfg.allowed_kinds = {node::kind::struct_decl, node::kind::class_decl};
cfg.grab_tag_names = {"reflect"};

filter f(cfg);
for (auto& n : ast->find_descendants([](auto&){ return true; })) {
  f.add(n);
}
f.clean(); // Remove nodes that don't match config
```

**Automatic child inclusion:**
```cpp
filter::config cfg;
cfg.grab_tag_names = {"serialize"};
cfg.child_node_inclusion = filter::config::include; // Grab fields/methods too

filter f(cfg);
// Add only the tagged struct
f.add(struct_with_serialize_tag);
// Fields/methods are auto-added
```

**Multi-AST collection:**
```cpp
filter f(cfg);
for (auto& file : files) {
  auto ast = parser.parse(file.read(), args);
  for (auto& node : ast->get_children()) {
    f.add(node);
  }
}
// f.get_types() contains all unique matches across files
```

## Design Notes

**USR-based deduplication:** `add()` rejects nodes with USRs already in the filter. Same type parsed twice is stored once.

**Iterator support:** `filter` provides begin/end iterators for range-based loops.

**No ownership transfer:** Nodes added to filter are shared_ptr. Filter doesn't own the AST; dropping the root node invalidates filter contents.

**Clean vs. add:** `add()` checks `is_valid_type()` (kind + basic tag logic). `clean()` applies full `matches_config()` (parent/child rules). Call `clean()` after bulk insertion.

## Configuration Patterns

**Whitelist kinds, any tag:**
```cpp
cfg.allowed_kinds = {node::kind::enum_decl};
// No grab_tag_names = accept all enums
```

**Specific tag, exclude another:**
```cpp
cfg.grab_tag_names = {"reflect"};
cfg.avoid_tag_names = {"internal"};
```

**Parent propagation:**
```cpp
cfg.grab_tag_names = {"serialize"};
cfg.parent_node_inclusion = filter::config::include_recursively;
// Matched node + all ancestors added
```

## Performance

**add() cost:** O(n) scan of USR list (linear search). For large filters (>1000 nodes), this is slow. Consider pre-filtering with `find_descendants()`.

**Alternative:** Use `std::unordered_set<std::string>` of USRs externally for O(1) deduplication if performance matters.

## Typical Use Case

**Cross-file type collection for code generators:**
```cpp
// Parse all headers
std::vector<node_ptr> asts;
for (auto& header : headers) {
  asts.push_back(parser.parse(read_file(header), args));
}

// Collect all @reflect types
filter::config cfg;
cfg.grab_tag_names = {"reflect"};
cfg.allowed_kinds = {node::kind::struct_decl, node::kind::class_decl};

filter types(cfg);
for (auto& ast : asts) {
  for (auto& node : ast->get_children()) {
    types.add(node);
  }
}
types.clean();

// Generate code
for (auto& type : types) {
  // ...
}
```
