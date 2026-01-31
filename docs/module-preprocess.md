# xccmeta_preprocess.hpp

## Purpose

**Optional module.** Exposes preprocessed C/C++ source text (macro-expanded, conditionals evaluated, includes resolved). The parser already preprocesses internally—this module is only for accessing the *expanded text itself*.

## Why It Exists

libclang's parser preprocesses automatically. You don't need this module for parsing. Use it when:
- Debugging macro expansions
- Emitting `#line` directives mapped to original source
- Displaying preprocessed code to users

**Critical:** Parsing does NOT require preprocessing input. The `parser` class handles it.

## Core Abstractions

**`preprocessor_context`** - Stateful preprocessor cache
- Stores macro definitions and include state
- Reusable across multiple `apply()` calls
- Constructor: `preprocessor_context(input, args)`

**`preprocessor`** - File-based preprocessor
- Constructor: `preprocessor(file(s), args)`
- `get_preprocessed_content()` - Returns expanded source per file
- `get_context()` - Returns reusable context

## When to Use

**Don't use for parsing:**
```cpp
// WRONG - parser already preprocesses
auto preprocessed = xccmeta::preprocessor(file, args).get_preprocessed_content()[0];
auto ast = parser.parse(preprocessed, args); // Redundant

// CORRECT
auto ast = parser.parse(file.read(), args); // Parser preprocesses internally
```

**Use for text inspection:**
```cpp
xccmeta::file f("input.hpp");
xccmeta::preprocessor pp(f, args);
auto expanded = pp.get_preprocessed_content()[0];

std::cout << "Macro-expanded source:\n" << expanded;
```

**Shared context across files:**
```cpp
// Define macros once, apply to multiple inputs
xccmeta::preprocessor_context ctx("#define PLATFORM_LINUX", args);

auto result1 = ctx.apply(file1.read(), args);
auto result2 = ctx.apply(file2.read(), args);
// Both files see PLATFORM_LINUX defined
```

## Design Notes

**Performance:** Preprocessing is as expensive as parsing (~1MB/s). Avoid unless necessary.

**Include resolution:** Follows same rules as `parser`. Relative includes resolve from CWD or `args.add_include_path()` directories.

**Output format:** Returns plain text with expanded macros, conditionals resolved, includes inlined. No AST structure.

**No source mapping:** Expanded text doesn't preserve original line numbers. Use libclang's source location APIs (in `parser`) for accurate diagnostics.

## Example: Macro Debugging

```cpp
const char* src = R"(
#define FOO(x) x * 2
#define BAR FOO(3) + FOO(5)

int value = BAR;
)";

xccmeta::compile_args args;
xccmeta::preprocessor_context ctx(src, args);
auto expanded = ctx.apply(src, args);

std::cout << expanded;
// Output: int value = 3 * 2 + 5 * 2;
```

## Comparison with Parser

| Feature | `parser` | `preprocessor` |
|---------|----------|----------------|
| Preprocesses input | Yes (internal) | Yes (output) |
| Returns | AST (`node_ptr`) | String |
| Use case | Metadata extraction | Text inspection |
| Required for parsing | Always | Never |

**Rule of thumb:** Use `parser` for 99% of workflows. Use `preprocessor` only when you need the expanded source text itself.

## Multi-File Context Sharing

**Use case:** Common header with macros included by multiple files.

```cpp
xccmeta::file common("common.hpp");
xccmeta::preprocessor_context ctx(common.read(), args);

for (auto& file : files) {
  auto expanded = ctx.apply(file.read(), args);
  // Process expanded content
}
```

**Limitation:** Context doesn't track include guards. Re-including same header multiple times may cause redefinition warnings.

## Design Rationale

**Why separate from parser?**
- Parser's preprocessing is opaque (libclang internal)
- Exposing text requires second preprocessing pass
- Most users don't need expanded text, so it's optional

**Pimpl pattern:** `preprocessor_context` uses `std::unique_ptr<internal_data>` for implementation hiding. Enables move-only semantics while keeping libclang types out of public API.

## When NOT to Use

**Avoid if:**
- You're just parsing code (use `parser` directly)
- You need AST metadata (preprocessing loses structure)
- Performance matters (preprocessing is slow)

**This module exists for edge cases.** Default to `parser::parse()` for standard workflows.
