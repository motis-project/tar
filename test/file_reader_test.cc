#include "doctest/doctest.h"

#include <iostream>

#include "tar/file_reader.h"

using namespace tar;

TEST_CASE("read file") {
  file_reader r{"../resources/test.bin"};
  auto content = r.read();

  bool content_check =
      (*content == "hello\nworld") || (*content == "hello\r\nworld");
  CHECK(content_check);
  CHECK(!r.read());
}