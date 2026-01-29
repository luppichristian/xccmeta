# Installation

This guide covers how to build and install xccmeta in your project.

## Requirements

- **CMake**: 3.19 or higher
- **Compiler**: C++20 compatible (GCC 10+, Clang 12+, MSVC 2019+)
- **LLVM/Clang**: 18.x (automatically fetched if not found)

## Building from Source

### Clone the Repository

```bash
git clone https://github.com/yourusername/xccmeta.git
cd xccmeta
```

### Configure with CMake

```bash
mkdir build
cd build
cmake ..
```

### Build

```bash
cmake --build .
```

### Run Tests (Optional)

```bash
ctest
```

## CMake Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `XCCMETA_BUILD_SHARED` | `OFF` | Build as a shared library instead of static |
| `XCCMETA_BUILD_TESTS` | `ON` | Build unit tests using Google Test |
| `XCCMETA_LLVM_FETCH` | `ON` | Fetch LLVM/Clang via FetchContent if not found |
| `XCCMETA_GOOGLE_TEST_FETCH` | `ON` | Fetch GoogleTest via FetchContent if not found |

### Example: Building as Shared Library

```bash
cmake -DXCCMETA_BUILD_SHARED=ON ..
cmake --build .
```

### Example: Without Tests

```bash
cmake -DXCCMETA_BUILD_TESTS=OFF ..
cmake --build .
```

## Using in Your Project

### Option 1: FetchContent (Recommended)

Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    xccmeta
    GIT_REPOSITORY https://github.com/yourusername/xccmeta.git
    GIT_TAG main
)

FetchContent_MakeAvailable(xccmeta)

target_link_libraries(your_target PRIVATE xccmeta::xccmeta)
```

### Option 2: find_package

After installing xccmeta system-wide:

```cmake
find_package(xccmeta REQUIRED)
target_link_libraries(your_target PRIVATE xccmeta::xccmeta)
```

### Option 3: Subdirectory

Clone or copy xccmeta into your project:

```cmake
add_subdirectory(external/xccmeta)
target_link_libraries(your_target PRIVATE xccmeta::xccmeta)
```

## Include Headers

```cpp
// Include everything
#include <xccmeta.hpp>

// Or include specific modules
#include <xccmeta/xccmeta_parser.hpp>
#include <xccmeta/xccmeta_node.hpp>
```

## Shared Library Symbol Visibility

When building as a shared library, xccmeta uses the `XCCMETA_API` macro for symbol visibility:

- On Windows: `__declspec(dllexport)` / `__declspec(dllimport)`
- On GCC/Clang: `__attribute__((visibility("default")))`

The `XCCMETA_SHARED` and `XCCMETA_EXPORTS` macros are automatically defined during the build process.
