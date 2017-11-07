#pragma once

#include <stdexcept>

namespace tar {

inline void verify(bool condition, char const* error = "verify failed") {
  if (!condition) {
    throw std::runtime_error(error);
  }
}

}  // namespace tar
