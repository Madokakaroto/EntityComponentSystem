#pragma once

#include <array>
#include <string_view>
#include <algorithm>

#ifndef _MSC_VER
#include <cxxabi.h>
#endif

namespace punk
{
#ifdef _MSC_VER
    inline constexpr std::array<std::string_view, 4> msvc_typename_decorator
    {{
        "struct ",
        "class ",
        "enum ",
        " "
    }};
#endif

    template <typename T>
    [[nodiscard]] inline std::string get_demangle_name()
    {
        decltype(auto) type_info = typeid(T);
#if defined(_MSC_VER)
        std::string result = type_info.name();
        for (auto const& decorator : msvc_typename_decorator)
        {
            for (auto itr = result.find(decorator); 
                 itr != std::string::npos; 
                 itr = result.find(decorator))
            {
                result.erase(itr, decorator.length());
            }
        }
        return result;

#elif defined(__clang__) || defined(__GNUC__)
        int status{ 0 };
        char* demangle_name = abi::__cxa_demangle(type_info.name(), nullptr, nullptr, &status);
        if (demangle_name)
        {
            std::string result{ demangle_name };
            std::free(demangle_name);
            return result;
        }
        throw std::runtime_error{ "Failed to demangle type name." };
#else
#error "Compiler Not Supported"
#endif
    }
}
