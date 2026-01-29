# Warnings

The `compile_warnings` class generates compiler warnings that appear when generated code is compiled. This provides compile-time feedback about metadata issues.

## Header

```cpp
#include <xccmeta/xccmeta_warnings.hpp>
```

## Overview

The compile warnings utility generates preprocessor directives that trigger compiler warnings:
- On MSVC: Uses `#pragma message`
- On GCC/Clang: Uses `#warning`

This allows code generators to surface issues (missing attributes, deprecated patterns, etc.) during the build process.

## Entry Structure

```cpp
struct entry {
    std::string message;
    source_location loc;
};
```

Each warning entry contains:
- `message` - The warning text
- `loc` - Optional source location for context

## Constructor

```cpp
compile_warnings();
```

Creates an empty warnings collection.

## Methods

### push

```cpp
compile_warnings& push(const std::string& message, source_location loc = {});
```

Add a warning message. Returns `*this` for chaining.

```cpp
warnings.push("Type 'Foo' has no serializable fields");
warnings.push("Deprecated attribute 'old_reflect'", loc);
```

### build

```cpp
std::string build() const;
```

Build the preprocessor directives string that will trigger warnings when compiled.

```cpp
std::string output = warnings.build();
```

### get_warnings

```cpp
const std::vector<entry>& get_warnings() const;
```

Get all warning entries.

## Basic Usage

```cpp
#include <xccmeta.hpp>
#include <iostream>
#include <fstream>

int main() {
    xccmeta::compile_warnings warnings;
    
    // Add warnings during code analysis
    warnings.push("Type 'Player' has no reflected fields");
    warnings.push("Consider using 'xccmeta::serialize' instead of deprecated 'reflect'");
    
    // Build warning directives
    std::string warning_code = warnings.build();
    
    // Write to generated file
    std::ofstream out("generated.cpp");
    out << warning_code;
    out << "\n// Rest of generated code...\n";
    
    return 0;
}
```

## Generated Output

The `build()` method generates cross-platform warning directives:

```cpp
// For a warning without location:
#if defined(_MSC_VER)
#pragma message("Warning: Type 'Player' has no reflected fields")
#elif defined(__GNUC__) || defined(__clang__)
#warning "Type 'Player' has no reflected fields"
#endif

// For a warning with location:
#if defined(_MSC_VER)
#pragma message("Warning: src/player.hpp:42: Type has no fields")
#elif defined(__GNUC__) || defined(__clang__)
#warning "src/player.hpp:42: Type has no fields"
#endif
```

## Integration with Generator

The `generator` class has built-in support for warnings:

```cpp
xccmeta::generator gen("output.cpp");

// Add warnings via generator
gen.warn("Missing attribute on field 'id'", field_node);
gen.warn("Type has no serializable members");

// Warnings are automatically included when done() is called
gen.done();
```

## Complete Example

```cpp
#include <xccmeta.hpp>
#include <iostream>
#include <fstream>

void validate_type(const xccmeta::node_ptr& type, xccmeta::compile_warnings& warnings) {
    // Check for empty types
    if (type->get_fields().empty() && type->get_methods().empty()) {
        warnings.push("Type '" + type->get_name() + "' is empty", 
                     type->get_location());
    }
    
    // Check for missing tags on fields
    for (const auto& field : type->get_fields()) {
        if (!field->has_tag("xccmeta::serialize") && 
            !field->has_tag("xccmeta::no_serialize")) {
            warnings.push("Field '" + field->get_name() + 
                         "' has no serialization attribute",
                         field->get_location());
        }
    }
    
    // Check for deprecated patterns
    if (type->has_tag("reflect")) {  // old-style tag
        warnings.push("Deprecated: Use 'xccmeta::reflect' instead of 'reflect'",
                     type->get_location());
    }
}

int main() {
    // Parse code
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        struct [[xccmeta::reflect]] Player {
            int id;  // Missing serialize attribute
            [[xccmeta::serialize]] float health;
        };
        
        struct [[reflect]] OldStyle {  // Deprecated
        };
    )", xccmeta::compile_args::modern_cxx());
    
    // Validate and collect warnings
    xccmeta::compile_warnings warnings;
    
    auto types = ast->find_descendants([](const auto& n) {
        return n->is_record_decl();
    });
    
    for (const auto& type : types) {
        validate_type(type, warnings);
    }
    
    // Output warnings
    std::cout << "=== Warnings (" << warnings.get_warnings().size() << ") ===\n";
    for (const auto& entry : warnings.get_warnings()) {
        std::cout << entry.message;
        if (entry.loc.is_valid()) {
            std::cout << " at " << entry.loc.to_string();
        }
        std::cout << "\n";
    }
    
    // Generate file with warnings
    std::ofstream out("generated.cpp");
    out << "// AUTO-GENERATED FILE\n\n";
    out << warnings.build();
    out << "\n// Generated code follows...\n";
    
    return 0;
}
```

## Use Cases

### Validation Feedback

```cpp
// Missing required attributes
warnings.push("Type must have 'component' tag for ECS registration", loc);

// Configuration issues
warnings.push("Field type 'void*' is not serializable");

// Deprecation notices
warnings.push("Attribute 'old_name' is deprecated, use 'new_name'");
```

### Documentation Reminders

```cpp
if (type->get_comment().empty()) {
    warnings.push("Type '" + type->get_name() + "' has no documentation");
}
```

### Build Configuration Checks

```cpp
#ifdef DEBUG
warnings.push("Debug build: reflection data includes private members");
#endif
```

## See Also

- [Generator](generator.md) - Using warnings with code generation
- [Source Location](source-location.md) - Location information for warnings
