#pragma once

#include <string>
#include <vector>

#include "xccmeta_base.hpp"

namespace xccmeta {

  class XCCMETA_API compile_args {
   public:
    compile_args();

    void add(const std::string& arg);
    const std::vector<std::string>& get_args() const;

   private:
    std::vector<std::string> args;
  };

}  // namespace xccmeta