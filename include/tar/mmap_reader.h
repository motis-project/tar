#pragma once

#ifdef _WIN32
#include "tar/mmap_reader_msvc.h"
#else
#include "tar/mmap_reader_unix.h"
#endif