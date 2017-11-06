#include "doctest.h"

#include <iostream>

#include "zstd-untar/zstd-untar.h"

using namespace zstd_untar;

TEST_CASE("dupsort") {
  printf("%zu\n", ZSTD_DStreamOutSize());

  auto r = file_reader{"../dir.tar"};

  auto f = read_file(r);
  CHECK(f);
  CHECK(*f == "hello\n");

  f = read_file(r);
  CHECK(f);
  CHECK(*f == "world\n");
}