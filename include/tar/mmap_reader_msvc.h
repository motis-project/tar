#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tar {

struct mmap_reader {
  struct file {
    explicit file(char const* path)
        : f_{CreateFileA(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL, nullptr)},
          size_{get_size(f_)} {
      if (f_ == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("cannot open file");
      }
    }

    file(file&& o) noexcept : f_{o.f_}, size_{o.size_} {
      o.f_ = INVALID_HANDLE_VALUE;
      o.size_ = 0;
    }

    file& operator=(file&& o) noexcept {
      f_ = o.f_;
      size_ = o.size_;
      o.f_ = INVALID_HANDLE_VALUE;
      o.size_ = 0;
      return *this;
    }

    file(file const&) = delete;
    file& operator=(file const&) = delete;

    ~file() {
      if (f_ != INVALID_HANDLE_VALUE) {
        CloseHandle(f_);
      }
    }

    static size_t get_size(HANDLE f) {
      if (f != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER filesize;
        GetFileSizeEx(f, &filesize);
        return filesize.QuadPart;
      } else {
        return 0;
      }
    }

    HANDLE f_;
    size_t size_;
  };

  struct memory_map {
    explicit memory_map(char const* path)
        : f_{path},
          fmap_{static_cast<char*>(
              mmap(nullptr, f_.size_, PROT_READ, MAP_PRIVATE, f_.f_, 0))} {
      if (fmap_ == MAP_FAILED) {  // NOLINT
        throw std::runtime_error("cannot memory map file");
      }
    }

    memory_map(memory_map&& o) noexcept
        : f_{std::move(o.f_)}, fmap_{std::move(o.fmap_)} {
      o.fmap_ = nullptr;
    }

    memory_map& operator=(memory_map&& o) noexcept {
      f_ = std::move(o.f_);
      fmap_ = o.fmap_;
      o.fmap_ = nullptr;
      return *this;
    }

    memory_map(memory_map const&) = delete;
    memory_map& operator=(memory_map const&) = delete;

    ~memory_map() {
      if (fmap_ != nullptr && fmap_ != MAP_FAILED) {  // NOLINT
        munmap(fmap_, f_.size_);
      }
    }

    static void* mmap(HANDLE* f) {
      auto const size = size();
      DWORD size_l = size;
      DWORD size_h = size >> 32u;
      return CreateFileMapping(f, nullptr, PAGE_READONLY, size_h, size_l, 0);
    }

    size_t size() const { return f_.size_; }
    char* ptr() const { return fmap_; }

    file f_;
    char* fmap_;
  };

  explicit mmap_reader(char const* filename) : m_(filename), it_(m_.ptr()) {}

  std::pair<char*, size_t> read(size_t const num_bytes) {
    auto const start = it_;
    auto const last = m_.ptr() + m_.size();
    auto const rest_size = static_cast<size_t>(last - start);
    auto const bytes_read = std::min(num_bytes, rest_size);
    it_ += bytes_read;
    return {start, bytes_read};
  }

  memory_map m_;
  char* it_;
};

}  // namespace tar
