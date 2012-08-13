#pragma once

// standard includes
#include <cstdint>

using byte_t = char;
static_assert(sizeof(byte_t) == 1, "A byte_t must be 1 byte long");
using ubyte_t = unsigned char;
static_assert(sizeof(ubyte_t) == 1, "A ubyte_t must be 1 byte long");

