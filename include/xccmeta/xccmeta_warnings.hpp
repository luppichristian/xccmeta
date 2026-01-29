#pragma once

#include "xccmeta_base.hpp"
#include "xccmeta_source.hpp"

namespace xccmeta {

  // This is an utility to output custom compile warnings.
  // It generates a string that contains definitions to trigger
  // warnings when compiled with specific compilers.
  // On MSVC it uses #pragma message, on GCC/Clang it uses #warning.
  class XCCMETA_API compile_warnings {
   public:
    compile_warnings() = default;

    compile_warnings& push(const std::string& message, source_location loc = {});
    std::string build() const;

   private:
    struct entry {
      std::string message;
      source_location loc;
    };

    std::vector<entry> warnings;
  };

}  // namespace xccmeta