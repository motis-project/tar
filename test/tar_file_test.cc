#include "doctest/doctest.h"

#include <iostream>

#include "tar/file_reader.h"
#include "tar/tar_reader.h"

using namespace tar;

TEST_CASE("untar tar") {
  tar_reader<file_reader> r{file_reader{"../resources/dir.tar"}};
  std::optional<std::string_view> s;
  std::string content;
  while ((s = r.read())) {
    content += *s;
  }
  CHECK(content == "hello\nworld\n");
}