#include "doctest.h"

#include <iostream>

#include "zstd-untar/zstd-untar.h"

using namespace zstd_untar;

TEST_CASE("untar") {

  SUBCASE(".tar file") {
    tar_reader r{file_reader{"../dir.tar"}};
    std::optional<std::string_view> s;
    std::string content;
    while ((s = r.read())) {
      content += *s;
    }
    CHECK(content == "hello\nworld\n");
  }

  SUBCASE(".tar.zst file") {
    tar_reader r{zstd_reader{"../dir.tar.zst"}};
    std::optional<std::string_view> s;
    std::string content;
    while ((s = r.read())) {
      content += *s;
    }
    CHECK(content == "hello\nworld\n");
  }
}