#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "tar/mmap_reader.h"

namespace tar {

struct file_reader {
  explicit file_reader(char const* s) : file_name_{s}, reader_{s} {}

  std::optional<std::string_view> read() { return read(reader_.m_.size()); }

  std::optional<std::string_view> read(size_t const s) {
    auto [ptr, size] = reader_.read(s);
    return size == 0 ? std::nullopt
                     : std::make_optional(std::string_view{ptr, size});
  }

  void skip(size_t const s) { reader_.read(s); }

  float progress() const { return reader_.progress(); }

  std::string_view current_file_name() const { return file_name_; }

  std::string file_name_;
  mmap_reader reader_;
};

}  // namespace tar
