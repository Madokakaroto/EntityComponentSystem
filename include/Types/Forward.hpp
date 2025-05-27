#pragma once

#include <cstdint>
#include <limits>
#include <concepts>
#include <compare>
#include <iostream>
#include <memory>
#include <array>
#include <tuple>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <type_traits>
#include <string>
#include <string_view>
#include <format>
#include <memory_resource>
#include <ranges>

namespace punk
{
    // arithmetic types
    using uint8_t  = std::uint8_t;
    using uint16_t = std::uint16_t;
    using uint32_t = std::uint32_t;
    using uint64_t = std::uint64_t;
    using sint8_t  = std::int8_t;
    using sint16_t = std::int16_t;
    using sint32_t = std::int32_t;
    using sint64_t = std::int64_t;
    using size_t   = std::size_t;

    // string types
    template <typename CharType, typename Traits = std::char_traits<CharType>>
    using basic_string = std::basic_string<CharType, Traits, std::pmr::polymorphic_allocator<CharType>>;

    using string = basic_string<char>;
    using wstring = basic_string<wchar_t>;
    using u8string = basic_string<char8_t>;
    using u16string = basic_string<char16_t>;
    using u32string = basic_string<char32_t>;

    // container types
    template <typename T>
    using vector = std::vector<T, std::pmr::polymorphic_allocator<T>>;

    template <typename Key, typename Value, typename Compare = std::less<Key>>
    using map = std::map<Key, Value, Compare, std::pmr::polymorphic_allocator<std::pair<Key const, Value>>>;

    template <typename Key, typename Value, typename Hash = std::hash<Key>, typename EqualTo = std::equal_to<Key>>
    using unordered_map = std::unordered_map<Key, Value, Hash, EqualTo, std::pmr::polymorphic_allocator<std::pair<Key const, Value>>>;
}