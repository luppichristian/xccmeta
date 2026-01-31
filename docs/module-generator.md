# xccmeta_generator.hpp

## Purpose

Utility for writing generated code to files with automatic indentation, separators, and warning emission.

## Why It Exists

Code generators need consistent formatting and diagnostic integration. `generator` provides a simple builder pattern for output construction without manual indent tracking or file management.

**Design goal:** Minimal API for structured text output. Not a template engine—just indented line emission.

## Core Abstractions

**`generator`** - Indented output writer
- Constructor takes output file path
- Auto-opens file, truncates existing content
- RAII: destructor closes file (or call `done()` explicitly)

**Methods:**
- `out(str)` - Emit line with current indentation
- `indent()` / `unindent()` - Adjust indentation level
- `separator()` - Emit `//` + 77 dashes
- `named_separator(name)` - Emit `// === name ===`
- `warn(msg, loc)` - Add compile-time warning

## When to Use

**Every code generation workflow:**
```cpp
xccmeta::generator gen("output.generated.hpp");

gen.out("#pragma once");
gen.out("");
gen.named_separator("Generated Serialization");

for (auto& type : types) {
  gen.out("struct " + type->get_name() + "_Serializer {");
  gen.indent();
  // ...
  gen.unindent();
  gen.out("};");
}

gen.done(); // Explicit close (optional)
```

**Indentation:**
```cpp
gen.out("namespace foo {");
gen.indent();
  gen.out("void bar() {");
  gen.indent();
    gen.out("// body");
  gen.unindent();
  gen.out("}");
gen.unindent();
gen.out("}");
```

**Warnings:**
```cpp
if (field->get_type().is_pointer()) {
  gen.warn("Pointer field may cause serialization issues", field);
}
// Emits #pragma message / #warning with file:line context
```

## Design Notes

**Indentation:** 2 spaces per level (hardcoded). No tab/width configuration.

**Separator width:** 77 characters. Matches common 80-column limit with comment prefix.

**Warning mechanism:** Uses `compile_warnings` internally. Generates preprocessor directives that trigger compiler warnings when output file is compiled.

**No buffering:** Each `out()` call writes to file immediately. Large outputs (>100k lines) may be slow. Consider batching if profiling shows file I/O overhead.

**Error handling:** File open failure is silent. `done()` returns `bool` indicating success. Check return value if output path may be invalid.

## Warning Output Format

**Generated warning code:**
```cpp
gen.warn("Unsupported type", node);

// Emits (MSVC):
#pragma message("file.hpp(42): warning: Unsupported type")

// Emits (GCC/Clang):
#warning "file.hpp:42: Unsupported type"
```

**Warnings compile into the generated file:** Users see them when building code that includes the output. Use for non-fatal issues (deprecated patterns, incomplete support).

## Typical Workflow

```cpp
// 1. Parse
auto ast = parser.parse(source, args);

// 2. Collect
auto types = ast->find_descendants(/* ... */);

// 3. Generate
generator gen("reflection.generated.hpp");

gen.out("#pragma once");
gen.out("#include <string>");
gen.out("");

for (auto& type : types) {
  gen.named_separator(type->get_name() + " Reflection");

  gen.out("template<>");
  gen.out("struct reflect<" + type->get_qualified_name() + "> {");
  gen.indent();

  gen.out("static constexpr const char* name = \"" + type->get_name() + "\";");
  gen.out("static constexpr size_t field_count = " + std::to_string(type->get_fields().size()) + ";");

  gen.unindent();
  gen.out("};");
  gen.out("");
}

if (!gen.done()) {
  std::cerr << "Failed to write output\n";
}
```

## Alternatives

**When NOT to use `generator`:**
- Complex templates (conditionals, loops within templates) → Use external template engine
- Multi-file output → Create multiple `generator` instances
- Non-line-based output (binary, JSON) → Use standard file I/O

**`generator` is optimal for:** Line-by-line C++ code emission with simple indentation.
