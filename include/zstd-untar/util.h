#pragma once

#include <stdexcept>

namespace zstd_untar {

inline void verify(bool condition, char const* error = "verify failed") {
  if (!condition) {
    throw std::runtime_error(error);
  }
}

}  // namespace zstd_untar