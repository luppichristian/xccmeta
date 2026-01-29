#pragma once

#include "xccmeta_base.hpp"

namespace xccmeta {

  // This is an utility to output custom compile warnings.
  // It generates a string that contains definitions to trigger
  // warnings when compiled with specific compilers.
  // On MSVC it uses #pragma message, on GCC/Clang it uses #warning.
  class compile_warnings {
   public:
    compile_warnings() = default;

    compile_warnings& push(const std::string& message);
    std::string build() const;

   private:
    std::vector<std::string> warnings;
  };

}  // namespace xccmeta