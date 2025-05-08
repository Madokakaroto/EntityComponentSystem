#include "ECS.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"

#include "Utils/StaticReflection.hpp"

struct foo {};
class fee {};


struct bar
{
    float float_value;
};

async_simple::coro::Lazy<int> get_43() {
    std::cout << "run with ::operator new/delete" << '\n';
    co_return 43;
}

int main(void)
{
    constexpr ecs::entity_t e{};
    static_assert(!e.is_valid());

    std::cout << "hello ecs" << std::endl;

    static_assert(ecs::murmur_hash_x86_32("hello ecs", 0x453627) != 0);

    std::cout << ecs::get_demangle_name<int>() << std::endl;
    std::cout << ecs::get_demangle_name<std::string>() << std::endl;
    std::cout << ecs::get_demangle_name<foo>() << std::endl;
    std::cout << ecs::get_demangle_name<fee>() << std::endl;

    syncAwait(get_43());


    static_assert(boost::pfr::tuple_size_v<bar> == 1);

    return 0;
}