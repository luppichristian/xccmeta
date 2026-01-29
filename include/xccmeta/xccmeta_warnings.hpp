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
    struct entry {
      std::string message;
      source_location loc;
    };

    compile_warnings() = default;

    compile_warnings& push(const std::string& message, source_location loc = {});
    std::string build() const;

    const std::vector<entry>& get_warnings() const;

   private:
    std::vector<entry> warnings;
  };

}  // namespace xccmeta