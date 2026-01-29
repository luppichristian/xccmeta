/*
MIT License

Copyright (c) 2026 Christian Luppi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "xccmeta/xccmeta_type_info.hpp"

#include <algorithm>
#include <sstream>

namespace xccmeta {

  type_info::type_info(const std::string& spelling): spelling_(spelling) {
  }

  // =============================================================================
  // Getters
  // =============================================================================

  const std::string& type_info::get_spelling() const {
    return spelling_;
  }
  const std::string& type_info::get_canonical() const {
    return canonical_;
  }
  bool type_info::is_const() const {
    return is_const_;
  }
  bool type_info::is_volatile() const {
    return is_volatile_;
  }
  bool type_info::is_restrict() const {
    return is_restrict_;
  }
  bool type_info::is_pointer() const {
    return is_pointer_;
  }
  bool type_info::is_reference() const {
    return is_reference_;
  }
  bool type_info::is_lvalue_reference() const {
    return is_lvalue_ref_;
  }
  bool type_info::is_rvalue_reference() const {
    return is_rvalue_ref_;
  }
  bool type_info::is_array() const {
    return is_array_;
  }
  bool type_info::is_function_pointer() const {
    return is_func_ptr_;
  }
  const std::string& type_info::get_pointee_type() const {
    return pointee_type_;
  }
  const std::string& type_info::get_array_element_type() const {
    return array_element_type_;
  }
  std::int64_t type_info::get_array_size() const {
    return array_size_;
  }
  std::int64_t type_info::get_size_bytes() const {
    return size_bytes_;
  }
  std::int64_t type_info::get_alignment() const {
    return alignment_;
  }
  bool type_info::is_valid() const {
    return !spelling_.empty();
  }
  bool type_info::is_arithmetic() const {
    return is_integral() || is_floating_point();
  }
  bool type_info::has_qualifiers() const {
    return is_const_ || is_volatile_ || is_restrict_;
  }

  // =============================================================================
  // Setters (protected)
  // =============================================================================

  void type_info::set_spelling(const std::string& s) {
    spelling_ = s;
  }
  void type_info::set_canonical(const std::string& c) {
    canonical_ = c;
  }
  void type_info::set_const(bool v) {
    is_const_ = v;
  }
  void type_info::set_volatile(bool v) {
    is_volatile_ = v;
  }
  void type_info::set_restrict(bool v) {
    is_restrict_ = v;
  }
  void type_info::set_pointer(bool v) {
    is_pointer_ = v;
  }
  void type_info::set_reference(bool v) {
    is_reference_ = v;
  }
  void type_info::set_lvalue_reference(bool v) {
    is_lvalue_ref_ = v;
  }
  void type_info::set_rvalue_reference(bool v) {
    is_rvalue_ref_ = v;
  }
  void type_info::set_array(bool v) {
    is_array_ = v;
  }
  void type_info::set_function_pointer(bool v) {
    is_func_ptr_ = v;
  }
  void type_info::set_pointee_type(const std::string& t) {
    pointee_type_ = t;
  }
  void type_info::set_array_element_type(const std::string& t) {
    array_element_type_ = t;
  }
  void type_info::set_array_size(std::int64_t s) {
    array_size_ = s;
  }
  void type_info::set_size_bytes(std::int64_t s) {
    size_bytes_ = s;
  }
  void type_info::set_alignment(std::int64_t a) {
    alignment_ = a;
  }

  // =============================================================================
  // Utility methods
  // =============================================================================

  bool type_info::is_void() const {
    const std::string& type = canonical_.empty() ? spelling_ : canonical_;
    return type == "void";
  }

  bool type_info::is_integral() const {
    const std::string& type = canonical_.empty() ? spelling_ : canonical_;

    // Check for common integral types
    static const char* integral_types[] = {
        "bool",
        "char",
        "signed char",
        "unsigned char",
        "char8_t",
        "char16_t",
        "char32_t",
        "wchar_t",
        "short",
        "short int",
        "signed short",
        "signed short int",
        "unsigned short",
        "unsigned short int",
        "int",
        "signed",
        "signed int",
        "unsigned",
        "unsigned int",
        "long",
        "long int",
        "signed long",
        "signed long int",
        "unsigned long",
        "unsigned long int",
        "long long",
        "long long int",
        "signed long long",
        "signed long long int",
        "unsigned long long",
        "unsigned long long int",
        // Fixed-width types (these might appear in canonical form)
        "int8_t",
        "int16_t",
        "int32_t",
        "int64_t",
        "uint8_t",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        "size_t",
        "ptrdiff_t",
        "intptr_t",
        "uintptr_t",
    };

    for (const char* integral : integral_types) {
      if (type == integral) return true;
    }
    return false;
  }

  bool type_info::is_floating_point() const {
    const std::string& type = canonical_.empty() ? spelling_ : canonical_;
    return type == "float" || type == "double" || type == "long double";
  }

  bool type_info::is_signed() const {
    const std::string& type = canonical_.empty() ? spelling_ : canonical_;

    // Unsigned types
    if (type.find("unsigned") != std::string::npos) return false;
    if (type == "bool") return false;
    if (type == "char8_t" || type == "char16_t" || type == "char32_t") return false;
    if (type.find("uint") == 0) return false;  // uint8_t, uint16_t, etc.
    if (type == "size_t" || type == "uintptr_t") return false;

    // Signed types
    if (type.find("signed") != std::string::npos) return true;
    if (type == "int" || type == "short" || type == "long" || type == "long long") return true;
    if (type.find("int") == 0 && type.find("_t") != std::string::npos) return true;  // int8_t, etc.
    if (type == "char") return true;                                                 // char is typically signed
    if (type == "ptrdiff_t" || type == "intptr_t") return true;
    if (type == "float" || type == "double" || type == "long double") return true;

    return false;
  }

  bool type_info::is_unsigned() const {
    const std::string& type = canonical_.empty() ? spelling_ : canonical_;

    if (type.find("unsigned") != std::string::npos) return true;
    if (type == "bool") return true;
    if (type == "char8_t" || type == "char16_t" || type == "char32_t" || type == "wchar_t") return true;
    if (type.find("uint") == 0) return true;
    if (type == "size_t" || type == "uintptr_t") return true;

    return false;
  }

  bool type_info::is_builtin() const {
    return is_void() || is_integral() || is_floating_point();
  }

  std::string type_info::get_unqualified_spelling() const {
    std::string result = spelling_;

    // Remove common qualifiers (simple approach)
    auto remove_word = [&result](const std::string& word) {
      size_t pos;
      while ((pos = result.find(word)) != std::string::npos) {
        result.erase(pos, word.length());
      }
    };

    remove_word("const ");
    remove_word(" const");
    remove_word("volatile ");
    remove_word(" volatile");
    remove_word("restrict ");
    remove_word(" restrict");
    remove_word("__restrict ");
    remove_word(" __restrict");

    // Trim whitespace
    auto ltrim = [](std::string& s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    };
    auto rtrim = [](std::string& s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    };

    ltrim(result);
    rtrim(result);

    return result;
  }

  std::string type_info::to_string() const {
    std::ostringstream oss;
    oss << "type_info{";
    oss << "spelling=\"" << spelling_ << "\"";

    if (!canonical_.empty() && canonical_ != spelling_) {
      oss << ", canonical=\"" << canonical_ << "\"";
    }

    if (is_const_) oss << ", const";
    if (is_volatile_) oss << ", volatile";
    if (is_restrict_) oss << ", restrict";

    if (is_pointer_) {
      oss << ", pointer";
      if (!pointee_type_.empty()) oss << " to \"" << pointee_type_ << "\"";
    }
    if (is_reference_) {
      oss << ", " << (is_lvalue_ref_ ? "lvalue" : "rvalue") << " reference";
      if (!pointee_type_.empty()) oss << " to \"" << pointee_type_ << "\"";
    }
    if (is_array_) {
      oss << ", array";
      if (array_size_ >= 0) oss << "[" << array_size_ << "]";
      if (!array_element_type_.empty()) oss << " of \"" << array_element_type_ << "\"";
    }
    if (is_func_ptr_) oss << ", function_pointer";

    if (size_bytes_ >= 0) oss << ", size=" << size_bytes_;
    if (alignment_ >= 0) oss << ", align=" << alignment_;

    oss << "}";
    return oss.str();
  }

}  // namespace xccmeta