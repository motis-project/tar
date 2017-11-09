#include "doctest.h"

#include "tar/tar_reader.h"
#include "tar/zstd_reader.h"

using namespace tar;

TEST_CASE("untar") {
  tar_reader<zstd_reader> r{zstd_reader{"../resources/dir.tar.zst"}};
  std::optional<std::string_view> s;
  std::string content;
  while ((s = r.read())) {
    content += *s;
  }
  CHECK(content == "hello\nworld\n");
}