#pragma once

#include <algorithm>
#include <optional>
#include <string_view>
#include <vector>

#include "zstd.h"

#include "zstd-untar/mmap_reader.h"

namespace zstd_untar {

inline void verify(bool condition, char const* error = "verify failed") {
  if (!condition) {
    throw std::runtime_error(error);
  }
}

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

struct zstd_reader {
  explicit zstd_reader(char const* s)
      : reader_{s},
        out_(ZSTD_DStreamOutSize()),
        out_fill_{0},
        prev_read_size_{0},
        dstream_{ZSTD_createDStream(), &ZSTD_freeDStream},
        next_to_read_{dstream_ ? ZSTD_initDStream(dstream_.get()) : 0} {
    verify(dstream_ != nullptr, "ZSTD_createDStream() error");
    verify(!ZSTD_isError(next_to_read_), "ZSTD_initDStream() error");
  }

  std::optional<std::string_view> read(size_t const n) {
    consume(n);
    read_to_out(n);
    return out_fill_ >= n ? std::make_optional(std::string_view{out_.data(), n})
                          : std::nullopt;
  }

  void read_to_out(size_t const min_size) {
    if (out_fill_ >= min_size) {
      return;
    }

    auto const out_buf_size =
        ZSTD_DStreamOutSize() *
        ((out_fill_ + 2 * ZSTD_DStreamOutSize() - 1) / ZSTD_DStreamOutSize());
    out_.resize(out_buf_size);

    while (true) {
      if (out_fill_ >= min_size) {
        break;
      }

      auto[buf_in, num_bytes_read] = reader_.read(next_to_read_);
      if (num_bytes_read == 0) {
        break;
      }

      auto input = ZSTD_inBuffer{buf_in, num_bytes_read, 0};
      while (input.pos < input.size) {
        auto output =
            ZSTD_outBuffer{out_.data() + out_fill_, ZSTD_DStreamOutSize(), 0};
        next_to_read_ = ZSTD_decompressStream(dstream_.get(), &output, &input);
        verify(!ZSTD_isError(next_to_read_), ZSTD_getErrorName(next_to_read_));
        out_fill_ += output.pos;
      }
    }
  }

  void skip(size_t const n) {
    consume(n);
    read_to_out(n);
  }

  void consume(size_t const next_read_size) {
    out_.erase(begin(out_), std::next(begin(out_), prev_read_size_));
    out_fill_ -= prev_read_size_;
    prev_read_size_ = next_read_size;
  }

  mmap_reader reader_;
  std::vector<char> out_;
  size_t out_fill_;
  size_t prev_read_size_;
  std::unique_ptr<ZSTD_DStream, decltype(&ZSTD_freeDStream)> dstream_;
  size_t next_to_read_;
};

inline int parse_oct(std::string_view s) {
  int i = 0;

  auto p = begin(s);
  auto n = s.size();
  while ((*p < '0' || *p > '7') && n > 0) {
    ++p;
    --n;
  }

  while (*p >= '0' && *p <= '7' && n > 0) {
    i *= 8;
    i += *p - '0';
    ++p;
    --n;
  }

  return i;
}

inline bool is_end_of_archive(std::string_view header) {
  verify(header.length() == 512);
  return std::all_of(begin(header), end(header),
                     [](auto&& c) { return c == 0; });
}

inline bool check_checksum(std::string_view p) {
  auto u = 0;
  for (auto n = 0; n < 512; ++n) {
    if (n < 148 || n > 155) {
      u += static_cast<unsigned char>(p[n]);
    } else {
      u += 0x20;
    }
  }
  return u == parse_oct(p.substr(148, 8));
}

inline bool is_file(std::string_view buf) {
  auto const file_type = buf[156];
  return file_type == 0 || file_type == '0';
}

inline int next_multiple_512(int n) {
  int a;
  a = n - 1;
  a = a >> 9;
  a = a + 1;
  return a << 9;
}

template <typename Reader>
struct tar_reader {
  explicit tar_reader(Reader&& r) : reader_{std::move(r)}, next_skip_{0} {}

  std::optional<std::string_view> read() {
    while (true) {
      reader_.skip(next_skip_);
      next_skip_ = 0;

      auto const opt_header = reader_.read(512);
      verify(opt_header.has_value(), "invalid end of archive");

      auto const header = *opt_header;
      if (is_end_of_archive(header)) {
        return {};
      }

      verify(header.size() == 512, "invalid tar file (size < 512)");
      verify(check_checksum(header), "invalid checksum");

      auto const file_size = parse_oct(header.substr(124, 12));
      auto const bytes_to_read = next_multiple_512(file_size);
      if (is_file(header)) {
        auto const buf = reader_.read(file_size);
        verify(buf.value_or("").size() == file_size, "invalid file in tar");
        next_skip_ = bytes_to_read - file_size;
        return buf;
      }

      reader_.skip(bytes_to_read);
    }
  }

  size_t next_skip_;
  Reader reader_;
};

}  // namespace zstd_untar