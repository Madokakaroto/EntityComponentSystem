#pragma once

#include <type_traits>
#include <format>
#include "TypeTraitsExt.hpp"

namespace ecs
{
    template <typename T>
    struct type_meta_traits
    {
        // static type check
        static_assert(std::conjunction_v<
            std::negation<std::is_reference<T>>,
            std::negation<std::is_pointer<T>>,
            std::negation<std::is_const<T>>,
            std::negation<std::is_volatile<T>>,
            std::negation<std::is_function<T>>,
            std::negation<std::is_void<T>>,
            std::negation<TIsCharArray<T>>
            >, "Type not Supported!"
        );


        using type = T;

    };
}