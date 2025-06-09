#pragma once

#include "Types/Entity.hpp"
#include "Types/ErrorCode.hpp"

namespace punk
{
    class store
    {
    public:
        store() = default;
        virtual ~store() = default;
        store(store const&) = delete;
        store& operator=(store const&) = delete;
        store(store&&) = delete;
        store& operator=(store&&) = delete;

    public:
    };
}