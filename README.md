# xccmeta

A C++ metadata extraction library built on libclang for code parsing, reflection, and build-time code generation.

xccmeta parses C/C++ source code into a queryable AST, extracts type information and custom
annotations, and provides utilities for generating boilerplate code such as serialization,
string conversions, and introspection routines.

## Features

- AST Parsing: Parse C/C++ source into a traversable Abstract Syntax Tree using libclang
- Type Introspection: Full type information including qualifiers, pointers, references, arrays, and size/alignment
- Tag Extraction: Extract custom metadata tags from comments (`/// @tag`) or clang attributes (`[[clang::annotate]]`)
- AST Filtering: Collect and deduplicate nodes by kind, tags, or custom criteria across multiple files
- Code Generation: Built-in generator utility for emitting formatted code with indentation and compiler warnings
- File Import: Wildcard-based file discovery and I/O for batch processing pipelines
- Preprocessing: Optional C/C++ preprocessing with macro expansion and include handling
- Cross-Platform: Windows, Linux, macOS support with static and shared library builds

## Requirements

- CMake 3.19+
- C++20 compatible compiler
- LLVM/Clang 18.x (automatically fetched via FetchContent if not found)
- GoogleTest (automatically fetched for tests if not found)

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `XCCMETA_BUILD_SHARED` | `OFF` | Build as shared library instead of static |
| `XCCMETA_BUILD_TESTS` | `ON` | Build unit tests using Google Test |
| `XCCMETA_BUILD_EXAMPLES` | `ON` | Build example programs |
| `XCCMETA_LLVM_FETCH` | `ON` | Fetch LLVM/Clang 18.x via FetchContent if not found |
| `XCCMETA_GOOGLE_TEST_FETCH` | `ON` | Fetch GoogleTest via FetchContent if not found |

## Documentation

See [docs](https://luppichristian.github.io/xccmeta/) for comprehensive API documentation and usage patterns.

## Examples

See the [examples/](examples/) directory for complete working examples.

## License

MIT License - see [LICENSE](LICENSE)
