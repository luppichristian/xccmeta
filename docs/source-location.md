# Source Location

The `source_location` and `source_range` structures provide source file position tracking for AST nodes and diagnostics.

## Header

```cpp
#include <xccmeta/xccmeta_source.hpp>
```

## source_location

Represents a single point in a source file.

### Fields

| Field | Type | Description |
|-------|------|-------------|
| `file` | `std::string` | File path |
| `line` | `std::uint32_t` | Line number (1-based) |
| `column` | `std::uint32_t` | Column number (1-based) |
| `offset` | `std::uint32_t` | Byte offset from file start |

### Constructors

```cpp
source_location();  // Default constructor
source_location(const std::string& file, std::uint32_t line, 
                std::uint32_t column = 1, std::uint32_t offset = 0);
```

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_valid()` | `bool` | Check if location is valid (non-zero line) |
| `same_file(other)` | `bool` | Check if same file as another location |
| `to_string()` | `std::string` | Format as "file:line:column" |
| `to_string_short()` | `std::string` | Format as "file:line" |

### Comparison Operators

```cpp
bool operator==(const source_location& other) const;
bool operator!=(const source_location& other) const;
bool operator<(const source_location& other) const;
```

### Example

```cpp
xccmeta::source_location loc("src/main.cpp", 42, 10);

if (loc.is_valid()) {
    std::cout << "Location: " << loc.to_string() << "\n";
    // Output: Location: src/main.cpp:42:10
}

// Short format (no column)
std::cout << loc.to_string_short() << "\n";
// Output: src/main.cpp:42
```

## source_range

Represents a range from start to end location.

### Fields

| Field | Type | Description |
|-------|------|-------------|
| `start` | `source_location` | Start of range |
| `end` | `source_location` | End of range |

### Static Factory Methods

#### from (two locations)

```cpp
static source_range from(const source_location& start, const source_location& end);
```

Create a range from start to end.

#### from (single location)

```cpp
static source_range from(const source_location& loc);
```

Create a range spanning a single location.

#### merge

```cpp
static source_range merge(const source_range& a, const source_range& b);
```

Merge two ranges into one that spans both.

### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_valid()` | `bool` | Check if range is valid |
| `is_empty()` | `bool` | Check if start == end |
| `contains(loc)` | `bool` | Check if location is within range |
| `contains(range)` | `bool` | Check if another range is fully within |
| `overlaps(range)` | `bool` | Check if ranges overlap |
| `length()` | `std::uint32_t` | Length in bytes (using offsets) |
| `to_string()` | `std::string` | Format as "file:start-end" |

### Comparison Operators

```cpp
bool operator==(const source_range& other) const;
bool operator!=(const source_range& other) const;
```

### Example

```cpp
xccmeta::source_location start("src/main.cpp", 10, 1);
xccmeta::source_location end("src/main.cpp", 15, 20);

auto range = xccmeta::source_range::from(start, end);

if (range.is_valid()) {
    std::cout << "Range: " << range.to_string() << "\n";
    std::cout << "Length: " << range.length() << " bytes\n";
}

// Check containment
xccmeta::source_location middle("src/main.cpp", 12, 5);
if (range.contains(middle)) {
    std::cout << "Middle is within range\n";
}
```

## Usage with Nodes

Every AST node has location information:

```cpp
auto node = /* ... */;

// Get declaration location (start point)
const auto& loc = node->get_location();
std::cout << "Declared at: " << loc.to_string() << "\n";

// Get full extent (start to end)
const auto& extent = node->get_extent();
std::cout << "Spans: " << extent.to_string() << "\n";
```

## Complete Example

```cpp
#include <xccmeta.hpp>
#include <iostream>

void analyze_locations(const xccmeta::node_ptr& ast) {
    auto types = ast->find_descendants([](const auto& n) {
        return n->is_record_decl();
    });
    
    for (const auto& type : types) {
        const auto& loc = type->get_location();
        const auto& extent = type->get_extent();
        
        std::cout << "Type: " << type->get_name() << "\n";
        std::cout << "  Declared at: " << loc.to_string() << "\n";
        std::cout << "  Extent: " << extent.to_string() << "\n";
        
        if (extent.is_valid()) {
            std::cout << "  Size: " << extent.length() << " bytes\n";
        }
        
        // Check if any fields overlap (shouldn't happen normally)
        auto fields = type->get_fields();
        for (size_t i = 0; i < fields.size(); ++i) {
            for (size_t j = i + 1; j < fields.size(); ++j) {
                if (fields[i]->get_extent().overlaps(fields[j]->get_extent())) {
                    std::cout << "  WARNING: Overlapping fields!\n";
                }
            }
        }
        
        std::cout << "\n";
    }
}

int main() {
    xccmeta::parser parser;
    auto ast = parser.parse(R"(
        struct Point {
            int x;
            int y;
        };
        
        class Rectangle {
        public:
            Point origin;
            int width;
            int height;
        };
    )", xccmeta::compile_args::modern_cxx());
    
    analyze_locations(ast);
    
    return 0;
}
```

## Merging Ranges

When combining declarations:

```cpp
std::vector<xccmeta::source_range> ranges;
// Collect ranges...

// Merge all into one spanning range
xccmeta::source_range combined = ranges[0];
for (size_t i = 1; i < ranges.size(); ++i) {
    combined = xccmeta::source_range::merge(combined, ranges[i]);
}

std::cout << "All declarations span: " << combined.to_string() << "\n";
```

## Comparison and Sorting

Locations can be compared and sorted:

```cpp
std::vector<xccmeta::node_ptr> nodes = /* ... */;

// Sort by location
std::sort(nodes.begin(), nodes.end(), 
    [](const auto& a, const auto& b) {
        return a->get_location() < b->get_location();
    });
```

## See Also

- [Node](node.md) - Accessing node locations
- [Warnings](warnings.md) - Using locations in warnings
- [Generator](generator.md) - Source-referenced code generation
