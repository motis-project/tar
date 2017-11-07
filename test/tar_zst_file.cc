#include "doctest.h"

#include <iostream>

#include "zstd-untar/tar_reader.h"
#include "zstd-untar/zstd_reader.h"

using namespace zstd_untar;

TEST_CASE("untar") {
  tar_reader r{zstd_reader{"../resources/dir.tar.zst"}};
  std::optional<std::string_view> s;
  std::string content;
  while ((s = r.read())) {
    content += *s;
  }
  CHECK(content == "hello\nworld\n");
}