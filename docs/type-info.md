# Type Info

The `type_info` class provides detailed information about C/C++ types, including qualifiers, pointers, references, arrays, and size information.

## Header

```cpp
#include <xccmeta/xccmeta_type_info.hpp>
```

## Overview

Every typed declaration (variables, fields, parameters, return types) has an associated `type_info` that describes its type in detail. This includes:

- The type name as written in source
- The canonical (fully resolved) type
- Type qualifiers (const, volatile, restrict)
- Pointer/reference information
- Array information
- Size and alignment

## Basic Type Names

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_spelling()` | `const std::string&` | Type name as written in source |
| `get_canonical()` | `const std::string&` | Canonical (fully resolved) type |
| `get_unqualified_spelling()` | `std::string` | Type name without qualifiers |

### Example

```cpp
// For: typedef int MyInt; const MyInt* ptr;
auto& type = node->get_type();
type.get_spelling();     // "const MyInt *"
type.get_canonical();    // "const int *"
type.get_unqualified_spelling(); // "MyInt *"
```

## Type Qualifiers

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_const()` | `bool` | Has const qualifier? |
| `is_volatile()` | `bool` | Has volatile qualifier? |
| `is_restrict()` | `bool` | Has restrict qualifier? (C) |
| `has_qualifiers()` | `bool` | Has any qualifier? |

### Example

```cpp
if (type.is_const()) {
    std::cout << "Type is const\n";
}

if (type.has_qualifiers()) {
    std::cout << "Type has qualifiers\n";
}
```

## Type Categories

### Pointer and Reference

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_pointer()` | `bool` | Is a pointer type? |
| `is_reference()` | `bool` | Is any reference type? |
| `is_lvalue_reference()` | `bool` | Is lvalue reference (&)? |
| `is_rvalue_reference()` | `bool` | Is rvalue reference (&&)? |
| `is_function_pointer()` | `bool` | Is function pointer? |
| `get_pointee_type()` | `const std::string&` | Type being pointed to |

### Example

```cpp
if (type.is_pointer()) {
    std::cout << "Pointer to: " << type.get_pointee_type() << "\n";
}

if (type.is_rvalue_reference()) {
    std::cout << "Rvalue reference to: " << type.get_pointee_type() << "\n";
}
```

### Arrays

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_array()` | `bool` | Is an array type? |
| `get_array_element_type()` | `const std::string&` | Element type of array |
| `get_array_size()` | `std::int64_t` | Array size (-1 if unknown) |

### Example

```cpp
if (type.is_array()) {
    std::cout << "Array of " << type.get_array_element_type();
    if (type.get_array_size() > 0) {
        std::cout << "[" << type.get_array_size() << "]";
    }
    std::cout << "\n";
}
```

## Built-in Type Categories

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_void()` | `bool` | Is void type? |
| `is_integral()` | `bool` | Is integral type (int, char, etc.)? |
| `is_floating_point()` | `bool` | Is floating point (float, double)? |
| `is_arithmetic()` | `bool` | Is arithmetic (integral or floating)? |
| `is_signed()` | `bool` | Is signed type? |
| `is_unsigned()` | `bool` | Is unsigned type? |
| `is_builtin()` | `bool` | Is a built-in type? |

### Example

```cpp
if (type.is_integral()) {
    if (type.is_unsigned()) {
        std::cout << "Unsigned integer type\n";
    } else {
        std::cout << "Signed integer type\n";
    }
}
```

## Size and Alignment

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_size_bytes()` | `std::int64_t` | Size in bytes (-1 if unknown) |
| `get_alignment()` | `std::int64_t` | Alignment in bytes (-1 if unknown) |

### Example

```cpp
auto size = type.get_size_bytes();
auto align = type.get_alignment();

if (size > 0) {
    std::cout << "Size: " << size << " bytes\n";
    std::cout << "Alignment: " << align << " bytes\n";
}
```

## Validity

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_valid()` | `bool` | Is the type info valid? |

### Example

```cpp
if (!type.is_valid()) {
    std::cerr << "Warning: Invalid type information\n";
}
```

## String Representation

```cpp
std::string to_string() const;
```

Returns a debug string representation of the type info.

### Example

```cpp
std::cout << "Type details: " << type.to_string() << "\n";
```

## Complete Example

```cpp
#include <xccmeta.hpp>
#include <iostream>

void analyze_type(const xccmeta::type_info& type) {
    std::cout << "Type: " << type.get_spelling() << "\n";
    std::cout << "Canonical: " << type.get_canonical() << "\n";
    
    // Qualifiers
    if (type.is_const()) std::cout << "  - const\n";
    if (type.is_volatile()) std::cout << "  - volatile\n";
    
    // Category
    if (type.is_pointer()) {
        std::cout << "  - pointer to: " << type.get_pointee_type() << "\n";
    } else if (type.is_lvalue_reference()) {
        std::cout << "  - lvalue reference to: " << type.get_pointee_type() << "\n";
    } else if (type.is_rvalue_reference()) {
        std::cout << "  - rvalue reference to: " << type.get_pointee_type() << "\n";
    } else if (type.is_array()) {
        std::cout << "  - array of: " << type.get_array_element_type();
        if (type.get_array_size() > 0) {
            std::cout << " [" << type.get_array_size() << "]";
        }
        std::cout << "\n";
    }
    
    // Built-in classification
    if (type.is_builtin()) {
        std::cout << "  - built-in type\n";
        if (type.is_integral()) {
            std::cout << "    - integral (" 
                      << (type.is_signed() ? "signed" : "unsigned") << ")\n";
        } else if (type.is_floating_point()) {
            std::cout << "    - floating point\n";
        }
    }
    
    // Size info
    if (type.get_size_bytes() > 0) {
        std::cout << "  - size: " << type.get_size_bytes() << " bytes\n";
        std::cout << "  - alignment: " << type.get_alignment() << " bytes\n";
    }
}

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        struct Example {
            int value;
            const char* name;
            float data[10];
            int&& rref;
        };
    )", xccmeta::compile_args::modern_cxx());
    
    auto structs = ast->find_descendants([](const xccmeta::node_ptr& n) {
        return n->get_kind() == xccmeta::node::kind::struct_decl;
    });
    
    for (const auto& s : structs) {
        std::cout << "=== " << s->get_name() << " ===\n";
        for (const auto& field : s->get_fields()) {
            std::cout << "\nField: " << field->get_name() << "\n";
            analyze_type(field->get_type());
        }
    }
    
    return 0;
}
```

## Type Info for Different Declarations

| Declaration Type | Type Info Contains |
|-----------------|-------------------|
| Variable/Field | The variable's type |
| Function parameter | The parameter's type |
| Function (via `get_return_type()`) | The return type |
| Typedef/Type alias | The aliased type |
| Base specifier | The base class type |

## See Also

- [Node](node.md) - Accessing type info from nodes
- [Parser](parser.md) - Creating AST with type info
