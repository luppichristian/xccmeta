# Import

The `import` module provides file importing capabilities with wildcard/glob pattern support for batch processing of source files.

## Header

```cpp
#include <xccmeta/xccmeta_import.hpp>
```

## Overview

The import module consists of two classes:
- `file` - Represents a single file with read/write operations
- `importer` - Imports multiple files using wildcard patterns

## file Class

### Constructor

```cpp
explicit file(const path& path);
```

Create a file object from a filesystem path.

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_path()` | `const path&` | Get the file path |
| `exists()` | `bool` | Check if file exists |
| `read()` | `std::string` | Read entire file contents |
| `write(content)` | `bool` | Write content to file |

### Example

```cpp
xccmeta::file f("src/main.cpp");

if (f.exists()) {
    std::string content = f.read();
    std::cout << "File size: " << content.size() << " bytes\n";
}
```

## importer Class

### Constructor

```cpp
explicit importer(const std::string& wildcard);
```

Create an importer that matches files using a wildcard/glob pattern.

**Supported patterns:**
- `*` - Matches any characters in a single path component
- `**` - Matches any characters across path components (recursive)
- `?` - Matches a single character
- Direct file path - Imports that single file

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_files()` | `const std::vector<file>&` | Get all matched files |

### Wildcard Examples

```cpp
// Import a single file
xccmeta::importer single("src/main.cpp");

// Import all .hpp files in include/
xccmeta::importer headers("include/*.hpp");

// Import all .cpp files recursively in src/
xccmeta::importer sources("src/**/*.cpp");

// Import all header files in any subdirectory
xccmeta::importer all_headers("**/*.hpp");
```

## Complete Example

### Processing Multiple Files

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    // Import all header files
    xccmeta::importer importer("include/**/*.hpp");
    
    std::cout << "Found " << importer.get_files().size() << " files\n";
    
    for (const auto& file : importer.get_files()) {
        std::cout << "  - " << file.get_path().string() << "\n";
    }
    
    return 0;
}
```

### Parsing Multiple Files

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    // Configure compile args
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    args.add_include_path("include");
    
    // Import source files
    xccmeta::importer importer("src/**/*.hpp");
    
    // Parse each file
    xccmeta::parser parser;
    xccmeta::node_ptr merged_ast = nullptr;
    
    for (const auto& file : importer.get_files()) {
        if (!file.exists()) {
            std::cerr << "File not found: " << file.get_path() << "\n";
            continue;
        }
        
        std::string content = file.read();
        auto ast = parser.parse(content, args);
        
        if (merged_ast) {
            merged_ast = parser.merge(merged_ast, ast, args);
        } else {
            merged_ast = ast;
        }
        
        std::cout << "Parsed: " << file.get_path().filename() << "\n";
    }
    
    // Now process the merged AST
    if (merged_ast) {
        auto types = merged_ast->find_descendants([](const auto& n) {
            return n->is_record_decl();
        });
        
        std::cout << "\nFound " << types.size() << " types across all files\n";
    }
    
    return 0;
}
```

### Writing Generated Files

```cpp
#include <xccmeta.hpp>

int main() {
    // Read template
    xccmeta::file template_file("templates/reflection.template");
    std::string template_content = template_file.read();
    
    // Process and generate
    std::string generated = process_template(template_content);
    
    // Write output
    xccmeta::file output("generated/reflection.cpp");
    if (output.write(generated)) {
        std::cout << "Generated: " << output.get_path() << "\n";
    } else {
        std::cerr << "Failed to write output\n";
    }
    
    return 0;
}
```

## Type Alias

The module uses `std::filesystem::path`:

```cpp
using path = std::filesystem::path;
```

## Use Cases

### Batch Header Processing

```cpp
// Process all public headers
xccmeta::importer headers("include/mylib/*.hpp");

for (const auto& header : headers.get_files()) {
    process_header(header);
}
```

### Project-Wide Reflection

```cpp
// Gather all source files
xccmeta::importer sources("src/**/*.cpp");
xccmeta::importer headers("include/**/*.hpp");

std::vector<xccmeta::file> all_files;
for (const auto& f : sources.get_files()) all_files.push_back(f);
for (const auto& f : headers.get_files()) all_files.push_back(f);

// Parse and extract metadata from all files
```

### Conditional File Processing

```cpp
xccmeta::importer files("data/*.json");

for (const auto& file : files.get_files()) {
    if (file.get_path().stem().string().starts_with("test_")) {
        // Skip test files
        continue;
    }
    process_config(file);
}
```

## Path Operations

Since `file` uses `std::filesystem::path`, you can use all standard path operations:

```cpp
xccmeta::file f("src/module/file.cpp");
const auto& path = f.get_path();

path.filename();      // "file.cpp"
path.stem();          // "file"
path.extension();     // ".cpp"
path.parent_path();   // "src/module"
path.is_absolute();   // true/false
```

## See Also

- [Parser](parser.md) - Parsing imported files
- [Preprocess](preprocess.md) - Preprocessing imported files
- [Generator](generator.md) - Writing generated output
