# xccmeta

A C/C++ metadata extraction library using libclang, designed for code parsing, reflection, and build pipeline integration.

## Features

- **AST Parsing**: Parse C/C++ source code into a traversable Abstract Syntax Tree using libclang
- **Type Introspection**: Full type information including qualifiers, pointers, references, arrays, and size/alignment
- **Tag Extraction**: Extract custom tags from comments or clang attributes
- **File Import**: Wildcard-based file importing for batch processing
- **Preprocessing**: Optional C/C++ preprocessing with macro expansion and include handling
- **AST Filtering**: Filter and collect nodes by kind, tags, or custom criteria
- **Code Generation**: Built-in generator utility for outputting processed metadata
- **Cross-platform**: Windows, Linux, macOS support
- **Flexible Linking**: Available as both static and shared library

## Requirements

- CMake 3.19 or higher
- C++20 compatible compiler
- LLVM/Clang 18.x (automatically fetched if not found)

## Documentation

See [docs](https://luppichristian.github.io/xccmeta/) for comprehensive API documentation and usage patterns.

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `XCCMETA_BUILD_SHARED` | `OFF` | Build as a shared library instead of static |
| `XCCMETA_BUILD_TESTS` | `ON` | Build unit tests using Google Test |
| `XCCMETA_LLVM_FETCH` | `ON` | Fetch LLVM/Clang via FetchContent if not found |
| `XCCMETA_GOOGLE_TEST_FETCH` | `ON` | Fetch GoogleTest via FetchContent if not found |

## Building

```bash
git clone https://github.com/luppichristian/xccmeta.git
cd xccmeta
mkdir build && cd build
cmake ..
cmake --build .
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Christian Luppi
