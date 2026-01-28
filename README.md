# xccmeta

A static language parser for C/C++, useful for code preprocessing build steps.

## Features

- Static analysis and parsing of C/C++ source files
- Designed for integration into build pipelines
- Available as both static and shared library
- Cross-platform support (Windows, Linux, macOS)
- C++17 compatible

## Requirements

- CMake 3.19 or higher
- C++17 compatible compiler

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `XCCMETA_BUILD_SHARED` | `OFF` | Build as a shared library instead of static |
| `XCCMETA_BUILD_TESTS` | `ON` | Build unit tests using Google Test |

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Christian Luppi
