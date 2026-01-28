# xccmeta

A C/C++ metadata extraction library using libclang, designed for code preprocessing, reflection, and build pipeline integration.

## Features

- **File Import**: Wildcard-based file importing for batch processing
- **Preprocessing**: C/C++ preprocessing with macro expansion and include handling
- **Attribute Parsing**: Extract custom `[[attributes]]` for metadata-driven code generation
- **Struct Introspection**: (Coming soon) Enumerate struct/class members and types
- **Cross-platform**: Windows, Linux, macOS support
- **Flexible Linking**: Available as both static and shared library

## Requirements

- CMake 3.19 or higher
- C++20 compatible compiler
- LLVM/Clang (automatically fetched if not found)

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `XCCMETA_BUILD_SHARED` | `OFF` | Build as a shared library instead of static |
| `XCCMETA_BUILD_TESTS` | `ON` | Build unit tests using Google Test |
| `XCCMETA_LLVM_FETCH` | `ON` | Fetch LLVM/Clang via FetchContent if not found |
| `XCCMETA_GOOGLE_TEST_FETCH` | `ON` | Fetch GoogleTest via FetchContent if not found |

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Christian Luppi
