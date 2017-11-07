#include "doctest.h"

#include <iostream>

#include "zstd-untar/tar_reader.h"
#include "zstd-untar/file_reader.h"

using namespace zstd_untar;

TEST_CASE("untar") {
  tar_reader r{file_reader{"../dir.tar"}};
  std::optional<std::string_view> s;
  std::string content;
  while ((s = r.read())) {
    content += *s;
  }
  CHECK(content == "hello\nworld\n");
}