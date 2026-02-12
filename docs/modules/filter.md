---
layout: default
title: Filter
---

# Filter

## Overview

Collects unique AST nodes matching configurable criteria (kinds, tags, parent/child inclusion). Deduplicates by USR and provides bulk filtering operations.

## Why Use This?

When collecting nodes across multiple files or ASTs, you need deduplication and consistent filtering rules. `filter` encapsulates common patterns like kind whitelists, multi-tag logic, and parent propagation into a reusable config.

## Types

| Type | Description |
|------|-------------|
| `filter` | Container of unique `node_ptr` (deduplicated by USR) |
| `filter::config` | Filtering rules (kinds, tags, inclusion modes) |
| `filter::config::node_inclusion` | Enum: `exclude`, `include`, `include_recursively` |

### filter::config Fields

| Field | Type | Description |
|-------|------|-------------|
| `allowed_kinds` | `std::vector<node::kind>` | Whitelist of node kinds (empty = all) |
| `grab_tag_names` | `std::vector<std::string>` | Include nodes with any of these tags |
| `avoid_tag_names` | `std::vector<std::string>` | Exclude nodes with any of these tags |
| `child_node_inclusion` | `node_inclusion` | How to handle children of matched nodes |
| `parent_node_inclusion` | `node_inclusion` | How to handle parents of matched nodes |

## Functions

### `filter()`

Default constructor. No filtering rules.

### `filter(cfg)`

Construct with filtering configuration.

**Signature:** `filter(const config& cfg)`

### `add`

Add a node to the filter. Rejects duplicates by USR.

**Signature:** `bool add(const node_ptr& type)`

**Returns:** `true` if added, `false` if already exists or invalid

### `remove`

Remove a node from the filter.

**Signature:** `bool remove(const node_ptr& type)`

**Returns:** `true` if removed, `false` if not found

### `clean`

Remove all nodes that don't match the full config criteria.

**Signature:** `filter& clean()`

**Returns:** `*this` for chaining

**Notes:** Call after bulk insertion. `add()` performs basic validation; `clean()` applies the full config (parent/child rules).

### `contains`

Check if a node is in the filter (by USR).

**Signature:** `bool contains(const node_ptr& type) const`

### `clear`

Remove all nodes.

**Signature:** `filter& clear()`

### `size`

Get the number of nodes.

**Signature:** `std::size_t size() const`

### `empty`

Check if the filter is empty.

**Signature:** `bool empty() const`

### `get_types`

Get all collected nodes.

**Signature:** `const std::vector<node_ptr>& get_types() const`

### `get_config`

Get the current configuration.

**Signature:** `const config& get_config() const`

### `is_valid_type`

Check if a node passes basic kind/tag validation.

**Signature:** `bool is_valid_type(const node_ptr& type) const`

### `matches_config`

Check if a node matches the full config criteria.

**Signature:** `bool matches_config(const node_ptr& type) const`

**Notes:**
- Supports range-based `for` loops via `begin()` / `end()` iterators.
- `add()` cost is O(n) scan of USR list. For >1000 nodes, consider pre-filtering with `find_descendants()`.

**Example:**

```cpp
xccmeta::filter::config cfg;
cfg.allowed_kinds = {xccmeta::node::kind::struct_decl, xccmeta::node::kind::class_decl};
cfg.grab_tag_names = {"reflect"};

xccmeta::filter types(cfg);
for (auto& file : imp.get_files()) {
  auto ast = parser.parse(file.read(), args);
  for (auto& node : ast->get_children()) {
    types.add(node);
  }
}
types.clean();

for (auto& type : types) {
  // Process deduplicated types
}
```
