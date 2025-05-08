#include "ECS.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"

#include "Utils/StaticReflection.hpp"

class foo {};

struct fee
{
    double double_value;
    int int_value;
};

struct bar
{
    float float_value;
};
PUNK_REFLECT(bar, float_value);

async_simple::coro::Lazy<int> get_43() {
    std::cout << "run with ::operator new/delete" << '\n';
    co_return 43;
}

int main(void)
{
    constexpr punk::entity_t e{};
    static_assert(!e.is_valid());

    std::cout << "hello punk" << std::endl;

    static_assert(punk::murmur_hash_x86_32("hello punk", 0x453627) != 0);

    std::cout << punk::get_demangle_name<int>() << std::endl;
    std::cout << punk::get_demangle_name<std::string>() << std::endl;
    std::cout << punk::get_demangle_name<foo>() << std::endl;
    std::cout << punk::get_demangle_name<fee>() << std::endl;

    syncAwait(get_43());


    static_assert(boost::pfr::tuple_size_v<bar> == 1);
    static_assert(!punk::reflected<foo>);
    static_assert(punk::reflected<bar>);
    static_assert(punk::auto_reflectable<fee>);

    bar b{ 1.0f };
    punk::for_each_field(b, [](auto value) { std::cout << value << std::endl; });
    punk::for_each_field_name(b, [](std::string_view name) { std::cout << name << std::endl; });
    punk::for_each_field_and_name(b, [](std::string_view name, auto value) { std::cout << name << ":" << value << std::endl; });

    fee f{ 2.0, 3 };
    punk::for_each_field(f, [](auto value) { std::cout << value << std::endl; });
    punk::for_each_field_name(f, [](std::string_view name) { std::cout << name << std::endl; });
    punk::for_each_field_and_name(f, [](std::string_view name, auto value) { std::cout << name << ":" << value << std::endl; });

    return 0;
}