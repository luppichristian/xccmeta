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

#include "xccmeta/xccmeta_source.hpp"

#include <algorithm>
#include <sstream>

namespace xccmeta {

  // --- source_location implementation ---

  source_location::source_location(const std::string& file, std::uint32_t line, std::uint32_t column, std::uint32_t offset)
      : file(file), line(line), column(column), offset(offset) {
  }

  bool source_location::is_valid() const {
    return !file.empty() && line > 0;
  }

  bool source_location::same_file(const source_location& other) const {
    return file == other.file;
  }

  std::string source_location::to_string() const {
    std::ostringstream oss;
    oss << file << ":" << line << ":" << column;
    return oss.str();
  }

  std::string source_location::to_string_short() const {
    std::ostringstream oss;
    oss << file << ":" << line;
    return oss.str();
  }

  bool source_location::operator==(const source_location& other) const {
    return file == other.file && line == other.line && column == other.column &&
           offset == other.offset;
  }

  bool source_location::operator!=(const source_location& other) const {
    return !(*this == other);
  }

  bool source_location::operator<(const source_location& other) const {
    if (file != other.file) return file < other.file;
    if (line != other.line) return line < other.line;
    if (column != other.column) return column < other.column;
    return offset < other.offset;
  }

  // --- source_range implementation ---

  source_range source_range::from(const source_location& start,
                                  const source_location& end) {
    return source_range {start, end};
  }

  source_range source_range::from(const source_location& loc) {
    return source_range {loc, loc};
  }

  source_range source_range::merge(const source_range& a,
                                   const source_range& b) {
    if (!a.is_valid()) return b;
    if (!b.is_valid()) return a;
    // Ensure both are in the same file
    if (!a.start.same_file(b.start)) return a;

    source_location new_start = (a.start < b.start) ? a.start : b.start;
    source_location new_end = (b.end < a.end) ? a.end : b.end;
    return source_range {new_start, new_end};
  }

  bool source_range::is_valid() const {
    return start.is_valid() && end.is_valid();
  }

  bool source_range::is_empty() const {
    return start == end;
  }

  bool source_range::contains(const source_location& loc) const {
    if (!is_valid() || !loc.is_valid()) return false;
    if (!start.same_file(loc)) return false;
    return !(loc < start) && (loc < end || loc == end);
  }

  bool source_range::contains(const source_range& other) const {
    if (!is_valid() || !other.is_valid()) return false;
    return contains(other.start) && contains(other.end);
  }

  bool source_range::overlaps(const source_range& other) const {
    if (!is_valid() || !other.is_valid()) return false;
    if (!start.same_file(other.start)) return false;
    // Ranges overlap if neither is entirely before the other
    return !(end < other.start || end == other.start) &&
           !(other.end < start || other.end == start);
  }

  std::uint32_t source_range::length() const {
    if (!is_valid()) return 0;
    if (end.offset < start.offset) return 0;
    return end.offset - start.offset;
  }

  std::string source_range::to_string() const {
    std::ostringstream oss;
    oss << start.file << ":" << start.line << ":" << start.column << "-"
        << end.line << ":" << end.column;
    return oss.str();
  }

  bool source_range::operator==(const source_range& other) const {
    return start == other.start && end == other.end;
  }

  bool source_range::operator!=(const source_range& other) const {
    return !(*this == other);
  }

}  // namespace xccmeta