#pragma once

#include <cinttypes>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "zstd.h"

#include "tar/mmap_reader.h"
#include "tar/util.h"

namespace tar {

struct zstd_reader {
  static constexpr auto const MAX_CONSUMED_BUFFER = 1024 * 1024;

  explicit zstd_reader(char const* s)
      : reader_{s},
        out_(2 * ZSTD_DStreamOutSize() + MAX_CONSUMED_BUFFER),
        out_fill_{0},
        prev_read_size_{0},
        dstream_{ZSTD_createDStream(), &ZSTD_freeDStream},
        next_to_read_{dstream_ ? ZSTD_initDStream(dstream_.get()) : 0},
        offset_{0} {
    verify(dstream_ != nullptr, "ZSTD_createDStream() error");
    verify(!ZSTD_isError(next_to_read_), "ZSTD_initDStream() error");
  }

  std::optional<std::string_view> read(size_t const n) {
    consume(n);
    read_to_out(n);
    return out_fill_ >= n
               ? std::make_optional(std::string_view{out_.data() + offset_, n})
               : std::nullopt;
  }

  void read_to_out(size_t const min_size) {
    while (true) {
      if (out_fill_ >= min_size) {
        break;
      }

      auto [buf_in, num_bytes_read] = reader_.read(next_to_read_);
      if (num_bytes_read == 0) {
        break;
      }

      auto input = ZSTD_inBuffer{buf_in, num_bytes_read, 0};
      while (input.pos < input.size) {
        resize_buffer();
        auto output = ZSTD_outBuffer{out_.data() + offset_ + out_fill_,
                                     ZSTD_DStreamOutSize(), 0};
        next_to_read_ = ZSTD_decompressStream(dstream_.get(), &output, &input);
        verify(!ZSTD_isError(next_to_read_), ZSTD_getErrorName(next_to_read_));
        out_fill_ += output.pos;
      }
    }
  }

  void resize_buffer() {
    if (out_.size() - offset_ - out_fill_ >= ZSTD_DStreamOutSize()) {
      return;
    }

    out_.resize(offset_ + out_fill_ + 2 * ZSTD_DStreamOutSize());
  }

  void skip(size_t const n) {
    consume(n);
    read_to_out(n);
  }

  void consume(size_t const next_read_size) {
    offset_ += prev_read_size_;
    out_fill_ -= prev_read_size_;
    prev_read_size_ = next_read_size;

    if (offset_ > MAX_CONSUMED_BUFFER) {
      out_.erase(begin(out_),
                 std::next(begin(out_), static_cast<int64_t>(offset_)));
      offset_ = 0;
    }
  }

  float progress() const { return reader_.progress(); }

  mmap_reader reader_;
  std::vector<char> out_;
  size_t out_fill_;
  size_t prev_read_size_;
  std::unique_ptr<ZSTD_DStream, decltype(&ZSTD_freeDStream)> dstream_;
  size_t next_to_read_;
  size_t offset_;
};

}  // namespace tar
