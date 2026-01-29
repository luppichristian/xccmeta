#include "xccmeta_compile_args.hpp"

namespace xccmeta {

  compile_args::compile_args() {
    // Initialize with default arguments
    args = {
        "-std=c++17",
        "-x",
        "c++",
        "-D__XCCMETA__=1"};
  }

  void compile_args::add(const std::string& arg) {
    args.push_back(arg);
  }

  const std::vector<std::string>& compile_args::get_args() const {
    return args;
  }

}  // namespace xccmeta