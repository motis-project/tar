#include "doctest.h"

#include <iostream>

#include "tar/file_reader.h"

using namespace tar;

TEST_CASE("read file") {
  file_reader r{"../resources/test.txt"};
  auto content = r.read();
  CHECK(content == "hello\nworld\n");
  CHECK(!r.read());
}