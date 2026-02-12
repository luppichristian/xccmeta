---
layout: default
title: Import
---

# Import

## Overview

Wraps file I/O and glob-based file discovery. Converts file paths (including wildcards) into content strings for parsing.

## Why Use This?

Parsing requires source text, not file paths. `importer` handles the filesystem layer so code generators don't mix path resolution with AST logic.

## Types

| Type | Description |
|------|-------------|
| `file` | Single file wrapper with read/write/exists |
| `importer` | Multi-file collector using glob patterns |
| `path` | Alias for `std::filesystem::path` |

## Functions

### file

#### `file(path)`

Construct from a filesystem path.

**Signature:** `explicit file(const path& path)`

#### `get_path`

Get the file path.

**Signature:** `const path& get_path() const`

#### `exists`

Check if the file exists on disk.

**Signature:** `bool exists() const`

#### `read`

Read the entire file content as a string.

**Signature:** `std::string read() const`

**Returns:** file content

**Notes:** Throws `std::runtime_error` if the file doesn't exist or is unreadable.

#### `write`

Write content to the file (overwrites existing content).

**Signature:** `bool write(const std::string& content) const`

**Returns:** `true` on success, `false` on failure

### importer

#### `importer(wildcard)`

Construct and discover files matching a pattern.

**Signature:** `explicit importer(const std::string& wildcard)`

**Parameters:**
- `wildcard` — glob pattern or exact path

**Supported patterns:**
- `*.hpp` — all headers in current directory
- `src/**/*.cpp` — recursive glob
- `exact/path.hpp` — single file (no expansion)

#### `get_files`

Get all matched files.

**Signature:** `const std::vector<file>& get_files() const`

**Returns:** vector of `file` objects matching the pattern

**Notes:**
- Relative paths resolve from process working directory.
- Files are read as binary strings (UTF-8 assumed, no BOM handling).
- Each `read()` call hits disk; cache content if re-parsing the same files.

**Example:**

```cpp
// Single file
xccmeta::file f("input.hpp");
auto ast = parser.parse(f.read(), args);

// Multiple files
xccmeta::importer imp("include/**/*.hpp");
for (auto& file : imp.get_files()) {
  auto ast = parser.parse(file.read(), args);
}
```
