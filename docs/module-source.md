# xccmeta_source.hpp

## Purpose

Represents source locations and ranges for AST nodes. Enables source-accurate diagnostics, code generation with line directives, and location-based filtering.

## Why It Exists

libclang provides location data as opaque cursors. This module exposes structured, queryable location info decoupled from the parsing context.

**Use case:** Generated code can reference original source locations for debuggability. Error messages can pinpoint exact file:line:column.

## Core Abstractions

**`source_location`** - Single point in source
- `file` - Absolute path or virtual name
- `line`, `column` - 1-indexed
- `offset` - Byte offset from file start

**Methods:**
- `is_valid()` - Non-empty file, line > 0
- `same_file(other)` - Compare file paths
- `to_string()` - "file:line:column"

**`source_range`** - Span between two locations
- `start`, `end` - Both `source_location`

**Methods:**
- `from(loc)` - Single-point range
- `merge(a, b)` - Union of two ranges
- `contains(loc)`, `contains(range)` - Inclusion tests
- `overlaps(range)` - Intersection check
- `length()` - Byte span using offsets

## When to Use

**Diagnostic generation:**
```cpp
auto loc = node->get_location();
std::cerr << loc.to_string() << ": error: missing annotation\n";
```

**#line directives:**
```cpp
auto range = node->get_extent();
gen.out("#line " + std::to_string(range.start.line) + " \"" + range.start.file + "\"");
```

**Filter by location:**
```cpp
auto nodes = ast->find_descendants([](auto& n) {
  return n->get_location().file.ends_with("generated.hpp");
});
```

**Range-based code extraction** (requires original source text):
```cpp
auto range = node->get_extent();
auto snippet = src.substr(range.start.offset, range.length());
```

## Design Notes

**Virtual files:** Parser accepts in-memory strings. These have synthetic filenames like `"<input>"`. `source_location::file` preserves this.

**Comparison:** Locations are ordered lexicographically (file, then line, then column). Enables sorting nodes by source order.

**Offset reliability:** Only accurate when parsing with complete file content. In-memory parsing sets offsets relative to the input string start.

**No file I/O:** This module doesn't read files. It only stores location metadata extracted during parsing.
