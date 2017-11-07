#include "doctest.h"

#include <iostream>

#include "zstd-untar/zstd-untar.h"

using namespace zstd_untar;

TEST_CASE("dupsort") {
  //auto r = file_reader{"../dir.tar"};

  tar_reader r{zstd_reader{"20170101.tar.zst"}};
  for (auto f = r.read(); f; f = r.read()) {
    std::cout << "file: " << *f << "\n";
  }

  // auto f = read_file(r);
  // CHECK(f);
  // CHECK(*f == "hello\n");
  //
  // f = read_file(r);
  // CHECK(f);
  // CHECK(*f == "world\n");
}