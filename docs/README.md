# xccmeta Documentation

Welcome to the xccmeta documentation. xccmeta is a C/C++ metadata extraction library using libclang, designed for code parsing, reflection, and build pipeline integration.

## Table of Contents

### Getting Started
- [Installation](installation.md) - How to build and install xccmeta
- [Quick Start](quickstart.md) - Get up and running quickly

### Core Modules
- [Parser](parser.md) - Parse C/C++ source code into an AST
- [Node](node.md) - AST node representation and traversal
- [Type Info](type-info.md) - Type information and introspection
- [Tags](tags.md) - Metadata annotations via C++ attributes

### Utilities
- [Filter](filter.md) - Filter and collect AST nodes
- [Import](import.md) - Wildcard-based file importing
- [Preprocess](preprocess.md) - Optional C/C++ preprocessing
- [Compile Args](compile-args.md) - Compiler argument configuration
- [Generator](generator.md) - Code generation utilities
- [Warnings](warnings.md) - Compile-time warning generation
- [Source Location](source-location.md) - Source file location tracking

### Reference
- [API Reference](api-reference.md) - Complete API documentation
- [Examples](examples.md) - Usage examples

## Features

- **AST Parsing**: Parse C/C++ source code into a traversable Abstract Syntax Tree using libclang
- **Type Introspection**: Full type information including qualifiers, pointers, references, arrays, and size/alignment
- **Attribute Extraction**: Extract custom `[[attributes]]` (tags) for metadata-driven code generation
- **File Import**: Wildcard-based file importing for batch processing
- **Preprocessing**: Optional C/C++ preprocessing with macro expansion and include handling
- **AST Filtering**: Filter and collect nodes by kind, tags, or custom criteria
- **Code Generation**: Built-in generator utility for outputting processed metadata
- **Cross-platform**: Windows, Linux, macOS support

## Requirements

- CMake 3.19 or higher
- C++20 compatible compiler
- LLVM/Clang 18.x (automatically fetched if not found)

## Quick Example

```cpp
#include <xccmeta.hpp>

int main() {
    // Create compile arguments
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    
    // Parse source code
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        struct [[xccmeta::reflect]] MyClass {
            int value;
            void method();
        };
    )", args);
    
    // Find all reflected types
    auto reflected = ast->find_descendants([](const auto& node) {
        return node->has_tag("xccmeta::reflect");
    });
    
    return 0;
}
```

## License

This project is licensed under the MIT License.

## Author

Christian Luppi
