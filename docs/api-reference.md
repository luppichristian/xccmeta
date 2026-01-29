# API Reference

This page provides a complete API reference for all xccmeta classes and functions.

## Namespace

All xccmeta types are in the `xccmeta` namespace:

```cpp
namespace xccmeta { ... }
```

## Core Classes

### parser

```cpp
class parser {
public:
    parser();
    ~parser();
    
    parser(const parser&) = delete;
    parser& operator=(const parser&) = delete;
    parser(parser&&) noexcept = default;
    parser& operator=(parser&&) noexcept = default;
    
    std::shared_ptr<node> parse(const std::string& input, const compile_args& args);
    std::shared_ptr<node> merge(std::shared_ptr<node> a, std::shared_ptr<node> b, 
                                const compile_args& args);
};
```

### node

```cpp
class node : public std::enable_shared_from_this<node> {
public:
    enum class kind { /* see Node documentation */ };
    
    // Identity
    kind get_kind() const;
    const char* get_kind_name() const;
    static const char* kind_to_string(kind k);
    const std::string& get_usr() const;
    const std::string& get_name() const;
    const std::string& get_qualified_name() const;
    const std::string& get_display_name() const;
    const std::string& get_mangled_name() const;
    
    // Location
    const source_location& get_location() const;
    const source_range& get_extent() const;
    
    // Type info
    const type_info& get_type() const;
    const type_info& get_return_type() const;
    
    // Access/Storage
    access_specifier get_access() const;
    storage_class get_storage_class() const;
    
    // Properties (bool)
    bool is_definition() const;
    bool is_virtual() const;
    bool is_pure_virtual() const;
    bool is_override() const;
    bool is_final() const;
    bool is_static() const;
    bool is_const_method() const;
    bool is_inline() const;
    bool is_explicit() const;
    bool is_constexpr() const;
    bool is_noexcept() const;
    bool is_deleted() const;
    bool is_defaulted() const;
    bool is_anonymous() const;
    bool is_scoped_enum() const;
    bool is_template() const;
    bool is_template_specialization() const;
    bool is_variadic() const;
    bool is_bitfield() const;
    int get_bitfield_width() const;
    
    // Default values
    bool has_default_value() const;
    const std::string& get_default_value() const;
    
    // Enum
    const std::string& get_underlying_type() const;
    std::int64_t get_enum_value() const;
    
    // Base class
    bool is_virtual_base() const;
    
    // Comments
    const std::string& get_comment() const;
    const std::string& get_brief_comment() const;
    
    // Tags
    const std::vector<tag>& get_tags() const;
    bool has_tag(const std::string& name) const;
    bool has_tags(const std::vector<std::string>& names) const;
    std::optional<tag> find_tag(const std::string& name) const;
    std::vector<tag> find_tags(const std::vector<std::string>& names) const;
    std::vector<tag> get_parent_tags() const;
    std::vector<tag> get_all_tags() const;
    
    // Tree structure
    node_ptr get_parent() const;
    const std::vector<node_ptr>& get_children() const;
    
    template<typename Predicate> node_ptr find_child(Predicate pred) const;
    template<typename Predicate> std::vector<node_ptr> find_children(Predicate pred) const;
    template<typename Predicate> std::vector<node_ptr> find_descendants(Predicate pred) const;
    
    std::vector<node_ptr> get_children_by_kind(kind k) const;
    node_ptr find_child_by_name(const std::string& name) const;
    
    // Tag-based queries
    std::vector<node_ptr> get_children_by_tag(const std::string& tag_name) const;
    std::vector<node_ptr> get_children_by_tags(const std::vector<std::string>& tag_names) const;
    std::vector<node_ptr> get_children_without_tag(const std::string& tag_name) const;
    std::vector<node_ptr> get_children_without_tags(const std::vector<std::string>& tag_names) const;
    node_ptr find_child_with_tag(const std::string& tag_name) const;
    node_ptr find_child_with_tags(const std::vector<std::string>& tag_names) const;
    node_ptr find_child_without_tag(const std::string& tag_name) const;
    node_ptr find_child_without_tags(const std::vector<std::string>& tag_names) const;
    
    // Convenience
    bool is_type_decl() const;
    bool is_record_decl() const;
    bool is_callable() const;
    std::vector<node_ptr> get_bases() const;
    std::vector<node_ptr> get_methods() const;
    std::vector<node_ptr> get_fields() const;
    std::vector<node_ptr> get_parameters() const;
    std::vector<node_ptr> get_enum_constants() const;
};

using node_ptr = std::shared_ptr<node>;
using node_weak_ptr = std::weak_ptr<node>;
```

### type_info

```cpp
class type_info {
public:
    type_info() = default;
    
    const std::string& get_spelling() const;
    const std::string& get_canonical() const;
    std::string get_unqualified_spelling() const;
    
    bool is_const() const;
    bool is_volatile() const;
    bool is_restrict() const;
    bool has_qualifiers() const;
    
    bool is_pointer() const;
    bool is_reference() const;
    bool is_lvalue_reference() const;
    bool is_rvalue_reference() const;
    bool is_array() const;
    bool is_function_pointer() const;
    
    const std::string& get_pointee_type() const;
    const std::string& get_array_element_type() const;
    std::int64_t get_array_size() const;
    
    std::int64_t get_size_bytes() const;
    std::int64_t get_alignment() const;
    
    bool is_valid() const;
    bool is_void() const;
    bool is_integral() const;
    bool is_floating_point() const;
    bool is_arithmetic() const;
    bool is_signed() const;
    bool is_unsigned() const;
    bool is_builtin() const;
    
    std::string to_string() const;
};
```

### tag

```cpp
class tag {
public:
    tag() = default;
    tag(const std::string& name, const std::vector<std::string>& args);
    
    static tag parse(const std::string& to_parse);
    
    const std::string& get_name() const;
    const std::vector<std::string>& get_args() const;
    std::string get_args_combined() const;
    std::string get_full() const;
};
```

### compile_args

```cpp
class compile_args {
public:
    compile_args();
    
    void add(const std::string& arg);
    void add_many(const std::vector<std::string>& args_to_add);
    void clear();
    const std::vector<std::string>& get_args() const;
    
    compile_args& set_standard(language_standard std);
    compile_args& set_language(language_mode lang);
    compile_args& add_include_path(const std::string& path);
    compile_args& add_include_paths(const std::vector<std::string>& paths);
    compile_args& define(const std::string& name);
    compile_args& define(const std::string& name, const std::string& value);
    compile_args& define(const std::string& name, int value);
    compile_args& undefine(const std::string& name);
    compile_args& set_target(const std::string& triple);
    compile_args& set_pointer_size(int bits);
    
    static compile_args modern_cxx(language_standard std = language_standard::cxx20);
    static compile_args modern_c(language_standard std = language_standard::c17);
    static compile_args minimal();
};
```

## Utility Classes

### filter

```cpp
class filter {
public:
    struct config {
        enum node_inclusion { exclude, include, include_recursively };
        std::vector<node::kind> allowed_kinds;
        std::vector<std::string> grab_tag_names;
        std::vector<std::string> avoid_tag_names;
        node_inclusion child_node_inclusion = node_inclusion::exclude;
        node_inclusion parent_node_inclusion = node_inclusion::exclude;
    };
    
    filter(const config& cfg = {});
    
    filter& clean();
    bool contains(const node_ptr& type) const;
    bool add(const node_ptr& type);
    bool remove(const node_ptr& type);
    filter& clear();
    std::size_t size() const;
    bool empty() const;
    const std::vector<node_ptr>& get_types() const;
    const config& get_config() const;
    bool is_valid_type(const node_ptr& type) const;
    bool matches_config(const node_ptr& type) const;
    
    auto begin();
    auto end();
    auto begin() const;
    auto end() const;
};
```

### generator

```cpp
class generator {
public:
    generator(const std::string& output_file);
    ~generator();
    
    generator& out(const std::string& data);
    generator& indent();
    generator& unindent();
    generator& separator();
    generator& named_separator(const std::string& name);
    generator& warn(const std::string& message, source_location loc = {});
    generator& warn(const std::string& message, const node_ptr node);
    bool done();
};
```

### file

```cpp
class file {
public:
    explicit file(const path& path);
    
    const path& get_path() const;
    bool exists() const;
    std::string read() const;
    bool write(const std::string& content) const;
};
```

### importer

```cpp
class importer {
public:
    explicit importer(const std::string& wildcard);
    
    const std::vector<file>& get_files() const;
};
```

### compile_warnings

```cpp
class compile_warnings {
public:
    struct entry {
        std::string message;
        source_location loc;
    };
    
    compile_warnings() = default;
    
    compile_warnings& push(const std::string& message, source_location loc = {});
    std::string build() const;
    const std::vector<entry>& get_warnings() const;
};
```

### preprocessor_context

```cpp
class preprocessor_context {
public:
    preprocessor_context();
    ~preprocessor_context();
    preprocessor_context(const std::string& input, const compile_args& args = compile_args());
    
    preprocessor_context(preprocessor_context&&) noexcept;
    preprocessor_context& operator=(preprocessor_context&&) noexcept;
    
    std::string apply(const std::string& to_preprocess, 
                     const compile_args& args = compile_args()) const;
};
```

### preprocessor

```cpp
class preprocessor {
public:
    explicit preprocessor(const file& file, const compile_args& args = compile_args());
    explicit preprocessor(const std::vector<file>& files, const compile_args& args = compile_args());
    
    const std::vector<std::string>& get_preprocessed_content() const;
    const preprocessor_context& get_context() const;
};
```

## Structures

### source_location

```cpp
struct source_location {
    std::string file;
    std::uint32_t line = 0;
    std::uint32_t column = 0;
    std::uint32_t offset = 0;
    
    source_location() = default;
    source_location(const std::string& file, std::uint32_t line, 
                   std::uint32_t column = 1, std::uint32_t offset = 0);
    
    bool is_valid() const;
    bool same_file(const source_location& other) const;
    std::string to_string() const;
    std::string to_string_short() const;
    
    bool operator==(const source_location& other) const;
    bool operator!=(const source_location& other) const;
    bool operator<(const source_location& other) const;
};
```

### source_range

```cpp
struct source_range {
    source_location start;
    source_location end;
    
    static source_range from(const source_location& start, const source_location& end);
    static source_range from(const source_location& loc);
    static source_range merge(const source_range& a, const source_range& b);
    
    bool is_valid() const;
    bool is_empty() const;
    bool contains(const source_location& loc) const;
    bool contains(const source_range& other) const;
    bool overlaps(const source_range& other) const;
    std::uint32_t length() const;
    std::string to_string() const;
    
    bool operator==(const source_range& other) const;
    bool operator!=(const source_range& other) const;
};
```

## Enumerations

### language_standard

```cpp
enum class language_standard {
    c89, c99, c11, c17, c23,
    cxx98, cxx03, cxx11, cxx14, cxx17, cxx20, cxx23, cxx26
};
```

### language_mode

```cpp
enum class language_mode {
    c, cxx, objective_c, objective_cxx
};
```

### access_specifier

```cpp
enum class access_specifier {
    invalid, public_, protected_, private_
};
```

### storage_class

```cpp
enum class storage_class {
    none, extern_, static_, register_, auto_, thread_local_
};
```

## Free Functions

```cpp
const char* access_specifier_to_string(access_specifier a);
const char* storage_class_to_string(storage_class sc);
```

## Type Aliases

```cpp
using path = std::filesystem::path;
using node_ptr = std::shared_ptr<node>;
using node_weak_ptr = std::weak_ptr<node>;
```
