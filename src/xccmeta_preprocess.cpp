#include "xccmeta_preprocess.hpp"

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/ModuleLoader.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/TargetParser/Host.h>

#include <sstream>

namespace xccmeta {

  struct preprocessor_context::internal_data {
    std::string source_code;
    std::vector<std::string> stored_args;

    internal_data() = default;
    ~internal_data() = default;

    // Non-copyable
    internal_data(const internal_data&) = delete;
    internal_data& operator=(const internal_data&) = delete;
  };

  // Default constructor
  preprocessor_context::preprocessor_context() = default;

  // Destructor (must be defined where internal_data is complete)
  preprocessor_context::~preprocessor_context() = default;

  // Move constructor
  preprocessor_context::preprocessor_context(preprocessor_context&&) noexcept = default;

  // Move assignment
  preprocessor_context& preprocessor_context::operator=(preprocessor_context&&) noexcept = default;

  preprocessor_context::preprocessor_context(const std::string& input, const compile_args& args) {
    data = std::make_unique<internal_data>();
    data->source_code = input;
    data->stored_args = args.get_args();
  }

  // Helper function to run the actual preprocessing
  static std::string run_preprocessor(const std::string& source, const std::vector<std::string>& args) {
    using namespace clang;

    // Create diagnostics
    auto diag_opts = llvm::makeIntrusiveRefCnt<DiagnosticOptions>();
    auto diag_printer = std::make_unique<TextDiagnosticPrinter>(llvm::errs(), diag_opts.get());
    auto diag_ids = llvm::makeIntrusiveRefCnt<DiagnosticIDs>();
    DiagnosticsEngine diagnostics(diag_ids, diag_opts.get(), diag_printer.release());

    // Create language options
    LangOptions lang_opts;
    lang_opts.CPlusPlus = true;
    lang_opts.CPlusPlus17 = true;

    // Create target info
    auto target_opts = std::make_shared<TargetOptions>();
    target_opts->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo* target_info = TargetInfo::CreateTargetInfo(diagnostics, target_opts);
    if (!target_info) {
      return source;
    }

    // Create file manager and source manager
    FileSystemOptions fs_opts;
    FileManager file_mgr(fs_opts);
    SourceManager source_mgr(diagnostics, file_mgr);

    // Create the memory buffer from source
    auto buffer = llvm::MemoryBuffer::getMemBufferCopy(source, "input.cpp");

    // Create a FileID for the memory buffer
    auto file_id = source_mgr.createFileID(std::move(buffer));
    source_mgr.setMainFileID(file_id);

    // Create header search options
    auto header_search_opts = std::make_shared<HeaderSearchOptions>();

    // Parse the args for include paths
    for (size_t i = 0; i < args.size(); ++i) {
      const auto& arg = args[i];
      if (arg == "-I" && i + 1 < args.size()) {
        header_search_opts->AddPath(args[i + 1], frontend::Quoted, false, true);
        ++i;
      } else if (arg.rfind("-I", 0) == 0 && arg.size() > 2) {
        header_search_opts->AddPath(arg.substr(2), frontend::Quoted, false, true);
      } else if (arg == "-isystem" && i + 1 < args.size()) {
        header_search_opts->AddPath(args[i + 1], frontend::System, false, true);
        ++i;
      }
    }

    // Create header search
    HeaderSearch header_search(header_search_opts, source_mgr, diagnostics, lang_opts, target_info);

    // Create preprocessor options
    auto pp_opts = std::make_shared<PreprocessorOptions>();

    // Build predefines buffer for command-line macros
    std::string predefines;

    // Parse defines from args
    for (size_t i = 0; i < args.size(); ++i) {
      const auto& arg = args[i];
      std::string macro_def;

      if (arg == "-D" && i + 1 < args.size()) {
        macro_def = args[i + 1];
        ++i;
      } else if (arg.rfind("-D", 0) == 0 && arg.size() > 2) {
        macro_def = arg.substr(2);
      } else if (arg == "-U" && i + 1 < args.size()) {
        predefines += "#undef " + args[i + 1] + "\n";
        ++i;
        continue;
      } else if (arg.rfind("-U", 0) == 0 && arg.size() > 2) {
        predefines += "#undef " + arg.substr(2) + "\n";
        continue;
      } else {
        continue;
      }

      // Parse macro_def: NAME or NAME=VALUE
      auto eq_pos = macro_def.find('=');
      if (eq_pos == std::string::npos) {
        // Simple define without value
        predefines += "#define " + macro_def + "\n";
      } else {
        // Define with value
        std::string name = macro_def.substr(0, eq_pos);
        std::string value = macro_def.substr(eq_pos + 1);
        predefines += "#define " + name + " " + value + "\n";
      }
    }

    // Create a trivial module loader (we don't need module support)
    TrivialModuleLoader module_loader;

    // Create the preprocessor
    Preprocessor pp(pp_opts, diagnostics, lang_opts, source_mgr, header_search, module_loader, /*IILookup=*/nullptr);
    pp.Initialize(*target_info);

    // Set predefines for command-line macros
    pp.setPredefines(predefines);

    pp.EnterMainSourceFile();

    // Collect all tokens with macro expansion
    std::ostringstream result;
    Token tok;
    bool first = true;
    bool need_space = false;
    SourceLocation last_loc;

    pp.Lex(tok);
    while (tok.isNot(tok::eof)) {
      // Get the spelling of the token
      std::string spelling = pp.getSpelling(tok);

      // Check if we need whitespace/newlines between tokens
      if (!first) {
        SourceLocation cur_loc = tok.getLocation();
        if (cur_loc.isValid() && last_loc.isValid()) {
          unsigned cur_line = source_mgr.getSpellingLineNumber(cur_loc);
          unsigned last_line = source_mgr.getSpellingLineNumber(last_loc);

          if (cur_line > last_line) {
            // Add newlines for line breaks
            for (unsigned i = last_line; i < cur_line; ++i) {
              result << "\n";
            }
            need_space = false;
          } else if (need_space) {
            result << " ";
          }
        } else if (need_space) {
          result << " ";
        }
      }

      result << spelling;
      first = false;
      last_loc = tok.getEndLoc();

      // Most tokens need space separation
      need_space = !spelling.empty() &&
                   tok.isNot(tok::l_paren) && tok.isNot(tok::l_brace) &&
                   tok.isNot(tok::l_square) && tok.isNot(tok::period) &&
                   tok.isNot(tok::arrow);

      pp.Lex(tok);
    }

    return result.str();
  }

  std::string preprocessor_context::apply(const std::string& to_preprocess, const compile_args& args) const {
    if (!data) return to_preprocess;

    // Combine stored args with provided args
    std::vector<std::string> combined_args = data->stored_args;
    const auto& extra_args = args.get_args();
    combined_args.insert(combined_args.end(), extra_args.begin(), extra_args.end());

    return run_preprocessor(to_preprocess, combined_args);
  }

  preprocessor::preprocessor(const file& file, const compile_args& args) {
    const auto file_contents = file.read();

    // Initialize context from the single file's content
    context = preprocessor_context(file_contents, args);
    content.push_back(context.apply(file_contents, args));
  }

  preprocessor::preprocessor(const std::vector<file>& files, const compile_args& args) {
    // Concatenate all file contents to initialize context
    std::vector<std::string> file_contents;
    std::string combined_contents;
    for (const auto& f : files) {
      file_contents.push_back(f.read());
      combined_contents += f.read() + "\n";
    }

    // Initialize context from combined contents
    context = preprocessor_context(combined_contents, args);

    // Preprocess each file's content
    for (const auto& content_str : file_contents) {
      content.push_back(context.apply(content_str, args));
    }
  }

  const std::vector<std::string>& preprocessor::get_preprocessed_content() const {
    return content;
  }

  const preprocessor_context& preprocessor::get_context() const {
    return context;
  }

}  // namespace xccmeta