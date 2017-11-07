#pragma once

#include "zstd-untar/mmap_reader.h"

namespace zstd_untar {

struct file_reader {
  explicit file_reader(char const* s) : reader_{s} {}

  std::optional<std::string_view> read(size_t const s) {
    auto[ptr, size] = reader_.read(s);
    return size == 0 ? std::nullopt
                     : std::make_optional(std::string_view{ptr, size});
  }

  void skip(size_t const s) { reader_.read(s); }

  mmap_reader reader_;
};

}  // namespace zstd_untar