#pragma once

#include "xccmeta_compile_args.hpp"
#include "xccmeta_import.hpp"

namespace xccmeta {

  // This class holds any state needed during preprocessing.
  //
  class XCCMETA_API preprocessor_context {
   public:
    preprocessor_context();
    ~preprocessor_context();
    preprocessor_context(const std::string& input, const compile_args& args = compile_args());

    // Move operations (needed for Pimpl with unique_ptr)
    preprocessor_context(preprocessor_context&&) noexcept;
    preprocessor_context& operator=(preprocessor_context&&) noexcept;

    // Delete copy operations
    preprocessor_context(const preprocessor_context&) = delete;
    preprocessor_context& operator=(const preprocessor_context&) = delete;

    std::string apply(const std::string& to_preprocess, const compile_args& args = compile_args()) const;

   private:
    struct internal_data;
    std::unique_ptr<internal_data> data;
  };

  // Preprocessor for files, it operates in two steps:
  // 1. Produces a "preprocessor_context" from the input files. This context holds
  //    any state needed for preprocessing (e.g., macros, includes, etc).
  // 2. Uses the context to produce preprocessed content for each file.
  class XCCMETA_API preprocessor {
   public:
    explicit preprocessor(const file& file, const compile_args& args = compile_args());
    explicit preprocessor(const std::vector<file>& files, const compile_args& args = compile_args());

    const std::vector<std::string>& get_preprocessed_content() const;
    const preprocessor_context& get_context() const;

   private:
    std::vector<std::string> content;
    preprocessor_context context;
  };

}  // namespace xccmeta