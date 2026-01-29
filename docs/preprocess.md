# Preprocess

The preprocess module provides optional C/C++ preprocessing capabilities. This module is **completely optional** for most use cases.

## Header

```cpp
#include <xccmeta/xccmeta_preprocess.hpp>
```

## Important Note

> **The parser already handles preprocessing internally.** The parser uses libclang which performs full C/C++ preprocessing before parsing. You do NOT need to preprocess input before calling `parser::parse()`.
>
> This module is only useful when you need:
> - The preprocessed source text itself (for display or debugging)
> - To inspect macro expansions
> - To share preprocessing context across multiple files

## Classes

### preprocessor_context

Holds preprocessing state (macros, includes, etc.) that can be reused.

#### Constructors

```cpp
preprocessor_context();
preprocessor_context(const std::string& input, const compile_args& args = compile_args());
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `apply(to_preprocess, args)` | `std::string` | Apply context to preprocess text |

#### Example

```cpp
// Create context from one file (captures its macros)
xccmeta::preprocessor_context ctx(header_content, args);

// Apply that context to preprocess another file
std::string preprocessed = ctx.apply(source_content, args);
```

### preprocessor

Preprocesses one or more files and stores the results.

#### Constructors

```cpp
explicit preprocessor(const file& file, const compile_args& args = compile_args());
explicit preprocessor(const std::vector<file>& files, const compile_args& args = compile_args());
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_preprocessed_content()` | `const std::vector<std::string>&` | Get preprocessed content for each file |
| `get_context()` | `const preprocessor_context&` | Get the preprocessing context |

## Basic Usage

### Preprocessing a Single File

```cpp
#include <xccmeta.hpp>

int main() {
    xccmeta::file source("src/main.cpp");
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    args.add_include_path("include");
    
    xccmeta::preprocessor pp(source, args);
    
    const auto& content = pp.get_preprocessed_content();
    if (!content.empty()) {
        std::cout << "Preprocessed content:\n" << content[0] << "\n";
    }
    
    return 0;
}
```

### Preprocessing Multiple Files

```cpp
#include <xccmeta.hpp>

int main() {
    xccmeta::importer importer("src/*.cpp");
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    
    xccmeta::preprocessor pp(importer.get_files(), args);
    
    const auto& results = pp.get_preprocessed_content();
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "File " << i << " preprocessed size: " 
                  << results[i].size() << " bytes\n";
    }
    
    return 0;
}
```

### Sharing Preprocessing Context

```cpp
#include <xccmeta.hpp>

int main() {
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    
    // Create context from a header that defines macros
    std::string header = R"(
        #define VERSION 2
        #define PLATFORM_WINDOWS 1
        #define STRINGIFY(x) #x
    )";
    
    xccmeta::preprocessor_context ctx(header, args);
    
    // Apply context to preprocess source files
    std::string source = R"(
        #if VERSION >= 2
        const char* version = STRINGIFY(VERSION);
        #endif
        
        #ifdef PLATFORM_WINDOWS
        void windows_only() {}
        #endif
    )";
    
    std::string preprocessed = ctx.apply(source, args);
    std::cout << "Preprocessed:\n" << preprocessed << "\n";
    
    return 0;
}
```

## Move Semantics

`preprocessor_context` supports move operations (for Pimpl pattern with `unique_ptr`):

```cpp
xccmeta::preprocessor_context ctx1(content, args);
xccmeta::preprocessor_context ctx2 = std::move(ctx1);  // OK

// xccmeta::preprocessor_context ctx3 = ctx2;  // Error: non-copyable
```

## When to Use Preprocessing

### You DON'T need this module if:

- You just want to parse C/C++ code (use `parser::parse()` directly)
- You want macros and conditionals evaluated during parsing
- You're doing standard code analysis or reflection

### You DO need this module if:

- You want to display the preprocessed source code
- You need to debug macro expansions
- You're building a code viewer/editor with macro expansion
- You want to see what code the compiler actually sees

## Example: Debugging Macros

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    args.define("DEBUG", "1");
    args.define("MAX_SIZE", "100");
    
    std::string code = R"(
        #ifdef DEBUG
        void debug_log(const char* msg);
        #endif
        
        int buffer[MAX_SIZE];
    )";
    
    xccmeta::preprocessor_context ctx(code, args);
    std::string result = ctx.apply(code, args);
    
    std::cout << "=== Original ===\n" << code << "\n";
    std::cout << "=== Preprocessed ===\n" << result << "\n";
    
    return 0;
}
```

**Output:**
```
=== Original ===

        #ifdef DEBUG
        void debug_log(const char* msg);
        #endif
        
        int buffer[MAX_SIZE];

=== Preprocessed ===

        
        void debug_log(const char* msg);
        
        
        int buffer[100];
```

## See Also

- [Parser](parser.md) - Parsing (includes preprocessing)
- [Compile Args](compile-args.md) - Setting defines and includes
- [Import](import.md) - File importing
