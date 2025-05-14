#pragma once

#include <type_traits>
#include "async_simple/coro/Lazy.h"

namespace punk
{
    template<size_t I, size_t N, typename F>
    constexpr void static_for(F&& f)
    {
        static_assert(I <= N);
        if constexpr (I < N)
        {
            std::forward<F>(f).template operator()<I>();
            static_for<I + 1, N>(std::forward<F>(f));
        }
    }

    template <size_t I, size_t N, typename F>
    constexpr auto async_static_for(F&& f) -> async_simple::coro::Lazy<>
    {
        static_assert(I <= N);
        if constexpr (I < N)
        {
            co_await std::forward<F>(f).template operator()<I>();
            if constexpr(I + 1 < N)
            {
                co_await generate_static_for<I + 1, N>(std::forward<F>(f));
            }
        }
    }
}
