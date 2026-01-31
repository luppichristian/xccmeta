# xccmeta_compile_args.hpp

## Purpose

Manages compiler arguments passed to libclang for parsing. Controls language standard, preprocessor defines, include paths, and target configuration.

## Why It Exists

libclang requires explicit compiler flags to parse code correctly. This abstraction prevents raw argument string construction and ensures compatibility across platforms and compilers.

**Design rationale:** Parsing behavior depends on compiler settings (e.g., `sizeof(void*)` changes with `-m32`). This class encapsulates platform-specific flag generation.

## Core Abstractions

**`compile_args`** - Fluent builder for compiler arguments
- **Language standard:** `set_standard(language_standard::cxx20)`
- **Preprocessor:** `define("MACRO")`, `define("FLAG", "value")`, `undefine("X")`
- **Include paths:** `add_include_path("/usr/include")`
- **Target config:** `set_target("x86_64-linux-gnu")`, `set_pointer_size(64)`

**Enums:**
- `language_standard` - C89 through C++26
- `language_mode` - C, C++, Objective-C/C++

## When to Use

**Always required** when calling `parser::parse()`.

**Use presets** for common configurations:
- `compile_args::modern_cxx()` - C++20 defaults
- `compile_args::modern_c()` - C17 defaults
- `compile_args::minimal()` - No defaults, full control

**Customize when:**
- Parsing code with preprocessor dependencies (`#ifdef PLATFORM_LINUX`)
- Cross-compiling (target triple affects type sizes, alignment)
- Non-standard include paths

## Example

```cpp
// Preset
auto args = xccmeta::compile_args::modern_cxx(language_standard::cxx17);

// Custom
xccmeta::compile_args args;
args.set_standard(language_standard::cxx20)
    .define("DEBUG", "1")
    .define("PLATFORM_LINUX")
    .add_include_path("/opt/custom/include")
    .set_target("aarch64-linux-gnu");
```

## Design Notes

**Fluent interface:** Setters return `*this` for chaining.

**No validation:** Invalid flags are passed directly to libclang. Parse failures provide diagnostics.

**Immutability not enforced:** Arguments can be modified after construction. Reuse with caution across parse calls if state matters.
