# Parser

The `parser` class converts C/C++ source code into an Abstract Syntax Tree (AST) using libclang.

## Header

```cpp
#include <xccmeta/xccmeta_parser.hpp>
```

## Overview

The parser is the main entry point for processing C/C++ source code. It takes source code as a string and produces a tree of `node` objects representing the declarations in that code.

## Important: Preprocessor Handling

The parser internally uses libclang which performs **full C/C++ preprocessing** before parsing. This means:

- `#define` macros are expanded
- `#ifdef`/`#ifndef`/`#if`/`#elif`/`#else`/`#endif` conditionals are evaluated
- `#include` directives are processed
- Macros defined via `compile_args` (e.g., `args.define("FOO")`) are respected

**You do NOT need to preprocess the input before calling `parse()`.** The optional preprocessor module is only useful if you need the preprocessed source text itself.

## Class Reference

### Constructor

```cpp
parser();
```

Creates a new parser instance.

### Methods

#### parse

```cpp
std::shared_ptr<node> parse(const std::string& input, const compile_args& args);
```

Parse input source code with given compile arguments.

**Parameters:**
- `input` - The C/C++ source code to parse
- `args` - Compile arguments (language standard, includes, defines, etc.)

**Returns:** A shared pointer to the root node of the AST (a `translation_unit` node)

**Example:**
```cpp
xccmeta::parser parser;
xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

auto ast = parser.parse(R"(
    struct Point {
        int x, y;
    };
)", args);
```

#### merge

```cpp
std::shared_ptr<node> merge(std::shared_ptr<node> a, std::shared_ptr<node> b, const compile_args& args);
```

Merge two AST nodes (e.g., from multiple translation units) into a single tree.

**Parameters:**
- `a` - First AST root node
- `b` - Second AST root node
- `args` - Compile arguments used for resolution

**Returns:** A merged AST containing declarations from both inputs

**Example:**
```cpp
xccmeta::parser parser;
xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

auto ast1 = parser.parse("struct A { int x; };", args);
auto ast2 = parser.parse("struct B { int y; };", args);

auto merged = parser.merge(ast1, ast2, args);
// merged now contains both struct A and struct B
```

## Copy/Move Semantics

- **Non-copyable**: `parser` cannot be copied
- **Moveable**: `parser` can be moved

```cpp
xccmeta::parser p1;
// xccmeta::parser p2 = p1;        // Error: non-copyable
xccmeta::parser p3 = std::move(p1); // OK: moveable
```

## Complete Example

```cpp
#include <xccmeta.hpp>
#include <iostream>

int main() {
    // Configure compile arguments
    xccmeta::compile_args args;
    args.set_standard(xccmeta::language_standard::cxx20)
        .define("MY_MACRO", "1")
        .add_include_path("/usr/include");
    
    // Create parser
    xccmeta::parser parser;
    
    // Parse source code
    auto ast = parser.parse(R"(
        #ifdef MY_MACRO
        namespace mylib {
            class [[xccmeta::reflect]] MyClass {
            public:
                int value;
                void doSomething();
            };
        }
        #endif
    )", args);
    
    // The AST root is a translation_unit node
    std::cout << "Root kind: " << ast->get_kind_name() << "\n";
    
    // Traverse children
    for (const auto& child : ast->get_children()) {
        std::cout << "Child: " << child->get_kind_name() 
                  << " - " << child->get_name() << "\n";
    }
    
    return 0;
}
```

## Error Handling

The parser does not throw exceptions. If parsing fails, the returned AST may be empty or contain partial results. Check the node structure to determine if parsing was successful.

```cpp
auto ast = parser.parse(code, args);
if (!ast || ast->get_children().empty()) {
    // Handle parse failure
}
```

## Thread Safety

The `parser` class is **not thread-safe**. Do not share a single parser instance between threads. Create separate parser instances for concurrent parsing:

```cpp
// Safe: each thread has its own parser
std::thread t1([&code, &args]() {
    xccmeta::parser parser;
    auto ast = parser.parse(code, args);
});
```

## See Also

- [Node](node.md) - Understanding the AST structure
- [Compile Args](compile-args.md) - Configuring parser options
- [Filter](filter.md) - Filtering parsed nodes
