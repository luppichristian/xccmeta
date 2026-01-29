#include "xccmeta/xccmeta_compile_args.hpp"

namespace xccmeta {

  compile_args::compile_args() {
    // Initialize with sensible defaults for C++ preprocessing
    args = {
        "-std=c++17",
        "-x",
        "c++",
        "-D__XCCMETA__=1"};
  }

  void compile_args::add(const std::string& arg) {
    args.push_back(arg);
  }

  void compile_args::add_many(const std::vector<std::string>& args_to_add) {
    args.insert(args.end(), args_to_add.begin(), args_to_add.end());
  }

  void compile_args::clear() {
    args.clear();
  }

  const std::vector<std::string>& compile_args::get_args() const {
    return args;
  }

  // ===== Language Standard and Mode =====

  const char* compile_args::standard_to_string(language_standard std) {
    switch (std) {
      case language_standard::c89:   return "c89";
      case language_standard::c99:   return "c99";
      case language_standard::c11:   return "c11";
      case language_standard::c17:   return "c17";
      case language_standard::c23:   return "c23";
      case language_standard::cxx98: return "c++98";
      case language_standard::cxx03: return "c++03";
      case language_standard::cxx11: return "c++11";
      case language_standard::cxx14: return "c++14";
      case language_standard::cxx17: return "c++17";
      case language_standard::cxx20: return "c++20";
      case language_standard::cxx23: return "c++23";
      case language_standard::cxx26: return "c++26";
      default:                       return "c++17";
    }
  }

  const char* compile_args::language_to_string(language_mode lang) {
    switch (lang) {
      case language_mode::c:             return "c";
      case language_mode::cxx:           return "c++";
      case language_mode::objective_c:   return "objective-c";
      case language_mode::objective_cxx: return "objective-c++";
      default:                           return "c++";
    }
  }

  compile_args& compile_args::set_standard(language_standard std) {
    args.push_back(std::string("-std=") + standard_to_string(std));
    return *this;
  }

  compile_args& compile_args::set_language(language_mode lang) {
    args.push_back("-x");
    args.push_back(language_to_string(lang));
    return *this;
  }

  // ===== Include Paths =====

  compile_args& compile_args::add_include_path(const std::string& path) {
    args.push_back("-I" + path);
    return *this;
  }

  compile_args& compile_args::add_include_paths(const std::vector<std::string>& paths) {
    for (const auto& path : paths) {
      add_include_path(path);
    }
    return *this;
  }

  // ===== Preprocessor Definitions =====

  compile_args& compile_args::define(const std::string& name) {
    args.push_back("-D" + name);
    return *this;
  }

  compile_args& compile_args::define(const std::string& name, const std::string& value) {
    args.push_back("-D" + name + "=" + value);
    return *this;
  }

  compile_args& compile_args::define(const std::string& name, int value) {
    args.push_back("-D" + name + "=" + std::to_string(value));
    return *this;
  }

  compile_args& compile_args::undefine(const std::string& name) {
    args.push_back("-U" + name);
    return *this;
  }

  // ===== Target Configuration =====

  compile_args& compile_args::set_target(const std::string& triple) {
    args.push_back("--target=" + triple);
    return *this;
  }

  compile_args& compile_args::set_pointer_size(int bits) {
    if (bits == 32) {
      args.push_back("-m32");
    } else if (bits == 64) {
      args.push_back("-m64");
    }
    return *this;
  }

  // ===== Common Presets =====

  compile_args compile_args::modern_cxx(language_standard std) {
    compile_args args;
    args.clear();
    args.set_language(language_mode::cxx)
        .set_standard(std)
        .define("__XCCMETA__", 1);
    return args;
  }

  compile_args compile_args::modern_c(language_standard std) {
    compile_args args;
    args.clear();
    args.set_language(language_mode::c)
        .set_standard(std)
        .define("__XCCMETA__", 1);
    return args;
  }

  compile_args compile_args::minimal() {
    compile_args args;
    args.clear();
    return args;
  }

}  // namespace xccmeta