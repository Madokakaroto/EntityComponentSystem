#pragma once

#include <type_traits>
#include "async_simple/coro/Lazy.h"

namespace punk
{
    template<size_t I, size_t N, class F>
    constexpr void static_for(F&& f)
    {
        static_assert(I <= N);
        if constexpr (I < N)
        {
            std::forward<F>(f).template operator()<I>();
            static_for<I + 1, N>(f);
        }
    }
}
