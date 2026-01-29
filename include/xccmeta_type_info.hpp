#pragma once

#include <cstdint>
#include <string>

#include "xccmeta_base.hpp"

namespace xccmeta {

  class XCCMETA_API type_info {
    friend class parser;
    friend class node;

   public:
    type_info() = default;

    // Type name as written in source
    const std::string& get_spelling() const;

    // Canonical (fully resolved) type name
    const std::string& get_canonical() const;

    // Type qualifiers
    bool is_const() const;
    bool is_volatile() const;
    bool is_restrict() const;

    // Type categories
    bool is_pointer() const;
    bool is_reference() const;
    bool is_lvalue_reference() const;
    bool is_rvalue_reference() const;
    bool is_array() const;
    bool is_function_pointer() const;

    // For pointers/references, the pointed-to type
    const std::string& get_pointee_type() const;

    // For arrays, the element type and size
    const std::string& get_array_element_type() const;
    std::int64_t get_array_size() const;

    // Size in bytes (if available, -1 otherwise)
    std::int64_t get_size_bytes() const;

    // Alignment in bytes (if available, -1 otherwise)
    std::int64_t get_alignment() const;

    // Utility methods
    bool is_valid() const;
    bool is_void() const;
    bool is_integral() const;
    bool is_floating_point() const;
    bool is_arithmetic() const;
    bool is_signed() const;
    bool is_unsigned() const;
    bool is_builtin() const;
    bool has_qualifiers() const;

    // Get the unqualified type name (removes const/volatile/etc)
    std::string get_unqualified_spelling() const;

    // String representation for debugging
    std::string to_string() const;

   protected:  // (only accessible by parser)
    explicit type_info(const std::string& spelling);
    void set_spelling(const std::string& s);
    void set_canonical(const std::string& c);
    void set_const(bool v);
    void set_volatile(bool v);
    void set_restrict(bool v);
    void set_pointer(bool v);
    void set_reference(bool v);
    void set_lvalue_reference(bool v);
    void set_rvalue_reference(bool v);
    void set_array(bool v);
    void set_function_pointer(bool v);
    void set_pointee_type(const std::string& t);
    void set_array_element_type(const std::string& t);
    void set_array_size(std::int64_t s);
    void set_size_bytes(std::int64_t s);
    void set_alignment(std::int64_t a);

   private:
    std::string spelling_;
    std::string canonical_;
    bool is_const_ = false;
    bool is_volatile_ = false;
    bool is_restrict_ = false;
    bool is_pointer_ = false;
    bool is_reference_ = false;
    bool is_lvalue_ref_ = false;
    bool is_rvalue_ref_ = false;
    bool is_array_ = false;
    bool is_func_ptr_ = false;
    std::string pointee_type_;
    std::string array_element_type_;
    std::int64_t array_size_ = -1;
    std::int64_t size_bytes_ = -1;
    std::int64_t alignment_ = -1;
  };

}  // namespace xccmeta