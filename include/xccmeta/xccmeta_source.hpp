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

namespace xccmeta {

  // Source location information
  struct XCCMETA_API source_location {
    std::string file;
    std::uint32_t line = 0;
    std::uint32_t column = 0;
    std::uint32_t offset = 0;

    // Default constructor
    source_location() = default;

    // Construct from file, line, column, offset
    source_location(const std::string& file, std::uint32_t line, std::uint32_t column = 1, std::uint32_t offset = 0);

    // Check if location is valid
    bool is_valid() const;

    // Check if location is in the same file as another
    bool same_file(const source_location& other) const;

    // Format as "file:line:column"
    std::string to_string() const;

    // Format as "file:line" (without column)
    std::string to_string_short() const;

    // Comparison operators
    bool operator==(const source_location& other) const;
    bool operator!=(const source_location& other) const;
    bool operator<(const source_location& other) const;
  };

  // Source range (start to end location)
  struct XCCMETA_API source_range {
    source_location start;
    source_location end;

    // Create a source range from two locations
    static source_range from(const source_location& start,
                             const source_location& end);

    // Create a source range spanning a single location
    static source_range from(const source_location& loc);

    // Merge two ranges into one that spans both
    static source_range merge(const source_range& a, const source_range& b);

    // Check if range is valid
    bool is_valid() const;

    // Check if range is empty (start == end)
    bool is_empty() const;

    // Check if a location is contained within this range
    bool contains(const source_location& loc) const;

    // Check if another range is fully contained within this range
    bool contains(const source_range& other) const;

    // Check if this range overlaps with another
    bool overlaps(const source_range& other) const;

    // Get the length in bytes (using offsets)
    std::uint32_t length() const;

    // Format as "file:start_line:start_col-end_line:end_col"
    std::string to_string() const;

    // Comparison operators
    bool operator==(const source_range& other) const;
    bool operator!=(const source_range& other) const;
  };

}  // namespace xccmeta