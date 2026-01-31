# xccmeta_import.hpp

## Purpose

Wraps file I/O and glob-based file discovery. Converts file paths (including wildcards) into content strings for parsing.

## Why It Exists

Parsing requires source text, not file paths. `importer` handles the filesystem layer so code generators don't mix path resolution with AST logic.

**Design goal:** Thin filesystem abstraction. No caching, no virtual filesystem—just read/write/glob.

## Core Abstractions

**`file`** - Single file wrapper
- Constructor: `file(std::filesystem::path)`
- `get_path()` - Returns path
- `exists()` - Check existence
- `read()` - Returns file content as string
- `write(content)` - Overwrites file

**`importer`** - Multi-file collector
- Constructor: `importer(wildcard_pattern)`
- `get_files()` - Returns vector of matched `file` objects

**Wildcard support:**
- `*.hpp` - All headers in CWD
- `src/**/*.cpp` - Recursive glob
- `exact/path.hpp` - Single file (no expansion)

## When to Use

**Single file:**
```cpp
xccmeta::file f("input.hpp");
if (!f.exists()) { /* error */ }

auto content = f.read();
auto ast = parser.parse(content, args);
```

**Multiple files:**
```cpp
xccmeta::importer imp("include/**/*.hpp");

for (auto& file : imp.get_files()) {
  auto content = file.read();
  auto ast = parser.parse(content, args);
  // Process ast...
}
```

**Write output:**
```cpp
xccmeta::file out("generated.hpp");
out.write(generated_code);
```

## Design Notes

**Glob implementation:** Uses `std::filesystem::recursive_directory_iterator`. Patterns follow shell glob syntax (not regex).

**Path resolution:** Relative paths resolve from process CWD, not executable location.

**Read failures:** `read()` throws `std::runtime_error` if file doesn't exist or is unreadable. Wrap in try-catch if paths are user-provided.

**Write atomicity:** `write()` truncates file. No temp-file-and-rename pattern. Partial writes on failure may corrupt output.

**No encoding detection:** Files read as binary, returned as `std::string`. UTF-8 assumed. BOM handling not implemented.

## Typical Use Case

**Batch processing:**
```cpp
// Find all headers with reflection tags
xccmeta::importer headers("src/**/*.hpp");

std::vector<xccmeta::node_ptr> all_types;

for (auto& file : headers.get_files()) {
  auto src = file.read();
  auto ast = parser.parse(src, args);

  auto tagged = ast->find_descendants([](auto& n) {
    return n->is_record_decl() && n->has_tag("reflect");
  });

  all_types.insert(all_types.end(), tagged.begin(), tagged.end());
}

// Generate from all collected types
// ...
```

## Integration with Parser

**Pattern:**
```cpp
xccmeta::importer imp("*.hpp");
for (auto& f : imp.get_files()) {
  auto ast = parser.parse(f.read(), args);
  // ...
}
```

**Not:**
```cpp
// WRONG - parser doesn't take file paths
auto ast = parser.parse("file.hpp", args);
```

**Merge pattern:**
```cpp
xccmeta::importer imp("include/*.hpp");
auto files = imp.get_files();

auto ast = parser.parse(files[0].read(), args);
for (size_t i = 1; i < files.size(); ++i) {
  auto next = parser.parse(files[i].read(), args);
  ast = parser.merge(ast, next, args);
}
// ast now contains all declarations from all files
```

## Performance

**Glob cost:** O(n) directory traversal. Expect ~1ms per 1000 files on SSD. Not a bottleneck compared to parsing.

**Read cost:** Synchronous I/O. For large files (>10MB), consider async I/O or memory mapping (not provided).

**No caching:** Each `read()` call hits disk. Cache content if re-parsing same files.

## Error Handling

**File not found:**
```cpp
xccmeta::file f("missing.hpp");
try {
  auto content = f.read();
} catch (const std::runtime_error& e) {
  std::cerr << "Failed to read: " << e.what() << '\n';
}
```

**Empty glob:**
```cpp
xccmeta::importer imp("nonexistent/*.hpp");
if (imp.get_files().empty()) {
  // No matches
}
```

**Write failure:** `write()` returns `bool`. `false` indicates failure (permission denied, disk full, etc.).
