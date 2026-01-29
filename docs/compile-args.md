# Compile Args

The `compile_args` class provides a fluent interface for configuring compiler arguments used during parsing and preprocessing.

## Header

```cpp
#include <xccmeta/xccmeta_compile_args.hpp>
```

## Overview

Compile arguments control how the parser interprets source code:
- Language standard (C++17, C++20, C11, etc.)
- Language mode (C, C++, Objective-C)
- Include paths
- Preprocessor definitions
- Target configuration

## Language Standards

```cpp
enum class language_standard {
    c89,    c99,    c11,    c17,    c23,
    cxx98,  cxx03,  cxx11,  cxx14,  cxx17,  cxx20,  cxx23,  cxx26
};
```

## Language Modes

```cpp
enum class language_mode {
    c,
    cxx,
    objective_c,
    objective_cxx
};
```

## Constructor

```cpp
compile_args();
```

Creates compile arguments with default settings.

## Factory Methods

### modern_cxx

```cpp
static compile_args modern_cxx(language_standard std = language_standard::cxx20);
```

Create compile arguments configured for modern C++ development.

### modern_c

```cpp
static compile_args modern_c(language_standard std = language_standard::c17);
```

Create compile arguments configured for modern C development.

### minimal

```cpp
static compile_args minimal();
```

Create compile arguments with no defaults (completely empty).

### Examples

```cpp
// Modern C++20 (recommended for most projects)
auto args = xccmeta::compile_args::modern_cxx();

// C++23 features
auto args = xccmeta::compile_args::modern_cxx(xccmeta::language_standard::cxx23);

// Modern C
auto args = xccmeta::compile_args::modern_c();

// Start from scratch
auto args = xccmeta::compile_args::minimal();
```

## Language Configuration

### set_standard

```cpp
compile_args& set_standard(language_standard std);
```

Set the language standard.

```cpp
args.set_standard(xccmeta::language_standard::cxx20);
```

### set_language

```cpp
compile_args& set_language(language_mode lang);
```

Set the language mode.

```cpp
args.set_language(xccmeta::language_mode::cxx);
```

## Include Paths

### add_include_path

```cpp
compile_args& add_include_path(const std::string& path);
```

Add a single include path (`-I<path>`).

```cpp
args.add_include_path("include");
args.add_include_path("/usr/local/include");
```

### add_include_paths

```cpp
compile_args& add_include_paths(const std::vector<std::string>& paths);
```

Add multiple include paths at once.

```cpp
args.add_include_paths({"include", "src", "third_party"});
```

## Preprocessor Definitions

### define (name only)

```cpp
compile_args& define(const std::string& name);
```

Define a macro without a value (`-D<name>`).

```cpp
args.define("DEBUG");
args.define("ENABLE_LOGGING");
```

### define (name and string value)

```cpp
compile_args& define(const std::string& name, const std::string& value);
```

Define a macro with a string value (`-D<name>=<value>`).

```cpp
args.define("VERSION", "\"1.0.0\"");
args.define("CONFIG_FILE", "\"config.json\"");
```

### define (name and integer value)

```cpp
compile_args& define(const std::string& name, int value);
```

Define a macro with an integer value.

```cpp
args.define("MAX_PLAYERS", 64);
args.define("BUFFER_SIZE", 4096);
```

### undefine

```cpp
compile_args& undefine(const std::string& name);
```

Undefine a macro (`-U<name>`).

```cpp
args.undefine("NDEBUG");
```

## Target Configuration

### set_target

```cpp
compile_args& set_target(const std::string& triple);
```

Set the target triple. This affects `sizeof`, predefined macros, and other target-specific behavior.

```cpp
args.set_target("x86_64-pc-windows-msvc");
args.set_target("x86_64-unknown-linux-gnu");
args.set_target("aarch64-apple-darwin");
```

### set_pointer_size

```cpp
compile_args& set_pointer_size(int bits);
```

Set pointer size for cross-compilation (`-m32` or `-m64`).

```cpp
args.set_pointer_size(64);  // 64-bit
args.set_pointer_size(32);  // 32-bit
```

## Raw Argument Access

### add

```cpp
void add(const std::string& arg);
```

Add a raw compiler argument.

```cpp
args.add("-fno-exceptions");
args.add("-Wno-deprecated");
```

### add_many

```cpp
void add_many(const std::vector<std::string>& args_to_add);
```

Add multiple raw arguments.

```cpp
args.add_many({"-fno-rtti", "-fno-exceptions"});
```

### get_args

```cpp
const std::vector<std::string>& get_args() const;
```

Get all configured arguments.

### clear

```cpp
void clear();
```

Remove all arguments.

## Method Chaining

All configuration methods return `*this`, enabling fluent method chaining:

```cpp
xccmeta::compile_args args;
args.set_standard(xccmeta::language_standard::cxx20)
    .set_language(xccmeta::language_mode::cxx)
    .add_include_path("include")
    .add_include_path("src")
    .define("DEBUG")
    .define("VERSION", 2)
    .set_target("x86_64-pc-windows-msvc");
```

## Complete Example

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    // Option 1: Start from preset
    auto args1 = xccmeta::compile_args::modern_cxx();
    args1.add_include_path("include")
         .define("MY_PROJECT");
    
    // Option 2: Build from scratch
    xccmeta::compile_args args2;
    args2.set_standard(xccmeta::language_standard::cxx20)
         .set_language(xccmeta::language_mode::cxx)
         .add_include_paths({"include", "src", "third_party/json/include"})
         .define("DEBUG")
         .define("MAX_ENTITIES", 1000)
         .define("PROJECT_NAME", "\"MyGame\"")
         .set_target("x86_64-pc-windows-msvc")
         .set_pointer_size(64);
    
    // Option 3: Cross-compilation setup
    xccmeta::compile_args cross_args;
    cross_args.set_standard(xccmeta::language_standard::c17)
              .set_language(xccmeta::language_mode::c)
              .set_target("arm-none-eabi")
              .set_pointer_size(32)
              .define("EMBEDDED")
              .define("__ARM_ARCH", 7);
    
    // Use with parser
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        #ifdef DEBUG
        void debug_print(const char* msg);
        #endif
        
        struct Entity {
            int id;
        };
    )", args2);
    
    // Debug: print all arguments
    std::cout << "Compile arguments:\n";
    for (const auto& arg : args2.get_args()) {
        std::cout << "  " << arg << "\n";
    }
    
    return 0;
}
```

## Common Configurations

### Game Development

```cpp
auto args = xccmeta::compile_args::modern_cxx(xccmeta::language_standard::cxx20);
args.add_include_paths({"engine/include", "game/include"})
    .define("GAME_DEBUG", 1)
    .define("PLATFORM_WINDOWS");
```

### Embedded Development

```cpp
auto args = xccmeta::compile_args::modern_c(xccmeta::language_standard::c11);
args.set_target("arm-none-eabi")
    .set_pointer_size(32)
    .define("EMBEDDED")
    .define("NDEBUG");
```

### Library Development

```cpp
auto args = xccmeta::compile_args::modern_cxx();
args.add_include_path("include")
    .define("MYLIB_EXPORTS")
    .define("MYLIB_VERSION", "\"2.0.0\"");
```

## See Also

- [Parser](parser.md) - Using compile args with the parser
- [Preprocess](preprocess.md) - Using compile args with preprocessing
