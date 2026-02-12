---
layout: default
title: Source
---

# Source

## Overview

Represents source locations and ranges for AST nodes. Enables source-accurate diagnostics, code generation with line directives, and location-based filtering.

## Why Use This?

Generated code can reference original source locations for debuggability. Error messages can pinpoint exact `file:line:column` positions.

## Types

| Type | Description |
|------|-------------|
| `source_location` | Single point in source (file, line, column, offset) |
| `source_range` | Span between two source locations |

## Functions

### source_location

#### `source_location()`

Default constructor. Creates an invalid location.

#### `source_location(file, line, column, offset)`

Construct from components.

**Signature:** `source_location(const std::string& file, std::uint32_t line, std::uint32_t column = 1, std::uint32_t offset = 0)`

**Parameters:**
- `file` — file path or virtual name
- `line` — 1-indexed line number
- `column` — 1-indexed column number (default: 1)
- `offset` — byte offset from file start (default: 0)

#### `is_valid`

Check if the location points to a real position.

**Signature:** `bool is_valid() const`

**Returns:** `true` if file is non-empty and line > 0

#### `same_file`

Check if two locations are in the same file.

**Signature:** `bool same_file(const source_location& other) const`

**Returns:** `true` if file paths match

#### `to_string`

Format as `"file:line:column"`.

**Signature:** `std::string to_string() const`

#### `to_string_short`

Format as `"file:line"` (without column).

**Signature:** `std::string to_string_short() const`

### source_range

#### `from` (static)

Create a range from two locations or a single location.

**Signatures:**
- `static source_range from(const source_location& start, const source_location& end)`
- `static source_range from(const source_location& loc)` — single-point range

#### `merge` (static)

Merge two ranges into one that spans both.

**Signature:** `static source_range merge(const source_range& a, const source_range& b)`

#### `is_valid`

Check if the range has valid start and end.

**Signature:** `bool is_valid() const`

#### `is_empty`

Check if start equals end.

**Signature:** `bool is_empty() const`

#### `contains`

Check if a location or range is within this range.

**Signatures:**
- `bool contains(const source_location& loc) const`
- `bool contains(const source_range& other) const`

#### `overlaps`

Check if this range overlaps with another.

**Signature:** `bool overlaps(const source_range& other) const`

#### `length`

Get the byte span using offsets.

**Signature:** `std::uint32_t length() const`

**Example:**

```cpp
auto loc = node->get_location();
std::cerr << loc.to_string() << ": error: missing annotation\n";

auto range = node->get_extent();
gen.out("#line " + std::to_string(range.start.line) + " \"" + range.start.file + "\"");
```
