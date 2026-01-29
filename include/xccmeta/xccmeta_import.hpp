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

#pragma once

#include "xccmeta_base.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace xccmeta {

  using path = std::filesystem::path;

  // File class to represent a single file.
  // Its just a wrapper around std::filesystem::path.
  class XCCMETA_API file {
   public:
    explicit file(const path& path);
    const path& get_path() const;

    bool exists() const;
    std::string read() const;
    bool write(const std::string& content) const;

   private:
    path file_path;
  };

  // Importer class to import multiple files based on a wildcard.
  // By passing in a direct filepath, it will import that single file.
  class XCCMETA_API importer {
   public:
    explicit importer(const std::string& wildcard);
    const std::vector<file>& get_files() const;

   private:
    std::vector<file> files;
  };

}  // namespace xccmeta