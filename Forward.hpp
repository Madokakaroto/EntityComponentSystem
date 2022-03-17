#pragma once

#include <cstdint>
#include <limits>
#include <concepts>
#include <compare>
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <type_traits>
#include <string>
#include <string_view>
#include <format>

#include "boost/pfr.hpp"

namespace ecs
{
    using uint8_t  = std::uint8_t;
    using uint16_t = std::uint16_t;
    using uint32_t = std::uint32_t;
    using uint64_t = std::uint64_t;
    using sint8_t  = std::int8_t;
    using sint16_t = std::int16_t;
    using sint32_t = std::int32_t;
    using sint64_t = std::int64_t;
    using size_t   = std::size_t;
}