#pragma once

#include <cstdint>

enum class Block : std::uint8_t {
    AIR = 0,
    GRASS = 1,
    STONE = 2,
    WATER = 3,
};
