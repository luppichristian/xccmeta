# xccmeta_base.hpp

## Purpose

Provides `XCCMETA_API` macro for shared library symbol visibility and common includes used across all modules.

## Why It Exists

Shared library builds require explicit symbol export/import on Windows (`__declspec(dllexport/dllimport)`) and GCC (`__attribute__((visibility("default")))`). This header centralizes the platform-specific logic.

## Core Abstractions

**`XCCMETA_API`** - DLL export/import macro
- Defined as `__declspec(dllexport)` when building shared lib (MSVC/Windows)
- Defined as `__declspec(dllimport)` when consuming shared lib (MSVC/Windows)
- Defined as `__attribute__((visibility("default")))` on GCC/Clang
- Empty for static builds

**Common includes:**
- `<cstdint>`, `<memory>`, `<optional>`, `<string>`, `<vector>`

## When to Use

**Do NOT include directly.** This is an internal header. Include module-specific headers instead:
```cpp
#include <xccmeta.hpp> // Main header (includes all modules)
// OR
#include <xccmeta/xccmeta_parser.hpp> // Specific module
```

## Symbol Visibility

**Static build (default):**
```cmake
# XCCMETA_BUILD_SHARED=OFF (default)
# XCCMETA_API expands to nothing
# All symbols visible (standard static library)
```

**Shared build:**
```cmake
set(XCCMETA_BUILD_SHARED ON)
# XCCMETA_API exports symbols for library consumers
```

**Why it matters:** Shared libs hide symbols by default on GCC. `XCCMETA_API` marks public API for export. Private implementation symbols remain hidden.

## Design Notes

**Macro detection:**
- `XCCMETA_SHARED` defined when building/consuming shared lib
- `XCCMETA_EXPORTS` defined only when *building* the shared lib (not when consuming)
- Combination determines `dllexport` vs `dllimport` on Windows

**No ABI versioning:** Symbol names are unmangled C++ (no `extern "C"`). Breaking changes require version bump.

**Platform assumptions:**
- Windows: Assumes MSVC or MinGW (checks `_WIN32`/`_WIN64`)
- Unix: Assumes GCC 4+ or Clang (checks `__GNUC__`)

## Typical Build Scenarios

**Static lib (most common):**
```cmake
add_executable(my_tool main.cpp)
target_link_libraries(my_tool PRIVATE xccmeta-static)
# No special configuration needed
```

**Shared lib:**
```cmake
set(XCCMETA_BUILD_SHARED ON)
add_executable(my_tool main.cpp)
target_link_libraries(my_tool PRIVATE xccmeta)
# Ensure xccmeta.dll / libxccmeta.so is in runtime search path
```

**Install shared lib:**
```bash
cmake --install . --prefix /usr/local
# Installs libxccmeta.so + headers
# Consumers link with -lxccmeta
```

## Header Organization

**[xccmeta_base.hpp:26-56](../include/xccmeta/xccmeta_base.hpp#L26-L56):**
- Lines 26-45: `XCCMETA_API` platform detection
- Lines 50-56: Common includes

**Why common includes?**
- Reduces duplication across module headers
- Ensures consistent STL types (`std::string`, `std::shared_ptr`)
- Single point for C++ version compatibility

## C++ Standard Dependency

**Requires C++20:**
- `std::shared_ptr` for AST node ownership
- `std::optional` for nullable queries
- Modern STL algorithms

**Not used (yet):** C++20 modules, coroutines, ranges. Library remains header-based.

## Cross-Platform Notes

**Windows DLL hell:** Shared lib requires `XCCMETA_API` on *every* public class/function. Missing annotation = link error.

**Unix default visibility:** GCC hides symbols by default with `-fvisibility=hidden`. `XCCMETA_API` marks public API.

**Static builds avoid complexity:** Default to static linking unless you need shared libs for plugin systems or reduced binary size.
