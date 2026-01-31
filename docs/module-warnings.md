# xccmeta_warnings.hpp

## Purpose

Generates compiler-specific preprocessor directives that emit warnings when compiled. Integrates code generator diagnostics into the build process.

## Why It Exists

Generated code may have issues detectable only at generation time (incomplete type support, deprecated patterns, etc.). `compile_warnings` lets generators inject warnings into output files so users see diagnostics when compiling, not when running the generator.

**Design goal:** Turn generation-time issues into compile-time warnings with source location context.

## Core Abstractions

**`compile_warnings`** - Warning accumulator
- `push(message, loc)` - Add warning with optional source location
- `build()` - Generate preprocessor directive string
- `get_warnings()` - Access raw warning list

**`entry`** - Single warning
- `message` - Text displayed to user
- `loc` - Optional `source_location` for file:line context

## When to Use

**Integrated into `generator`:**
```cpp
xccmeta::generator gen("output.hpp");

if (type->is_template()) {
  gen.warn("Template types not fully supported", type);
}
// Warning injected into generated file
```

**Standalone use:**
```cpp
xccmeta::compile_warnings warnings;

for (auto& field : fields) {
  if (field->get_type().is_pointer()) {
    warnings.push("Raw pointers require manual serialization", field->get_location());
  }
}

// Emit at top of generated file
std::string warning_code = warnings.build();
gen.out(warning_code);
```

## Generated Output

**Example:**
```cpp
warnings.push("Deprecated pattern", source_location("foo.hpp", 42, 10));
auto code = warnings.build();
```

**MSVC output:**
```cpp
#if defined(_MSC_VER)
#pragma message("foo.hpp(42): warning: Deprecated pattern")
#endif
```

**GCC/Clang output:**
```cpp
#if defined(__GNUC__) || defined(__clang__)
#warning "foo.hpp:42: Deprecated pattern"
#endif
```

**Cross-compiler:** Both directives emitted. Compiler picks appropriate one.

## Design Notes

**Warning vs. error:** Generates warnings, not errors. Build succeeds but user is notified. Use for non-fatal issues.

**Location formatting:** Uses compiler-specific format for IDE integration (clickable errors in VS Code, Visual Studio, etc.).

**No deduplication:** Same warning can be pushed multiple times. Dedupe externally if needed.

**Build timing:** Warnings appear when generated file is *compiled*, not when generator runs. Users see them during normal builds.

## Integration Pattern

**Typical workflow:**
```cpp
xccmeta::generator gen("output.hpp");

// Generate code
for (auto& type : types) {
  if (!is_fully_supported(type)) {
    gen.warn("Partial support for " + type->get_name(), type);
  }

  // Emit code anyway
  gen.out("struct " + type->get_name() + "_Meta { /* ... */ };");
}

gen.done();
```

**User experience:**
```bash
$ clang++ main.cpp
output.hpp:15: warning: Partial support for CustomType
```

## Use Cases

**Unsupported type categories:**
```cpp
if (field->get_type().is_function_pointer()) {
  gen.warn("Function pointers not serializable", field);
}
```

**Deprecated tag usage:**
```cpp
if (node->has_tag("old_reflect")) {
  gen.warn("@old_reflect is deprecated, use @reflect", node);
}
```

**Incomplete metadata:**
```cpp
if (node->get_type().get_size_bytes() == -1) {
  gen.warn("Type size unavailable, runtime overhead increased", node);
}
```

**Platform-specific issues:**
```cpp
if (args.get_target().starts_with("wasm")) {
  gen.warn("WebAssembly target has limited reflection support", node);
}
```

## Design Rationale

**Why preprocessor directives?**
- Cross-compiler compatibility (MSVC, GCC, Clang)
- No runtime overhead (pure compile-time)
- IDE integration (warnings appear in problem lists)

**Why not stderr during generation?**
- Users may not run generator directly (build scripts)
- Warnings tied to code locations, not generator invocations
- Persistent (warnings survive build system caching)

**Limitation:** Non-standard compilers (Intel, PGI) may ignore directives. Warnings are best-effort.

## Alternatives

**When NOT to use:**
- Fatal errors (throw exceptions in generator instead)
- Runtime diagnostics (emit runtime checks in generated code)
- Verbose logging (use stderr during generation)

**Use `compile_warnings` for:** Non-fatal generation issues that users should see when building the output.
