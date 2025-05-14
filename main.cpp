#include "ECS.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "src/CoreTypes.h"

#include "Utils/StaticReflection.hpp"
#include "Types/RTTI.h"

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

struct alignas(8) test_align
{
    int int_value;
    char char_value;
    double double_value;
    std::string str_value;
};

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

    using type_info_traits_bar = punk::type_info_traits<bar>;
    static_assert(type_info_traits_bar::get_field_count() == 1);
    std::cout << "field offset: " << type_info_traits_bar::get_field_offset<0>() << std::endl;
    static_assert(std::is_same_v<decltype(type_info_traits_bar::get_field_type<0>()), float>);

    using type_info_traits_fee = punk::type_info_traits<fee>;
    static_assert(type_info_traits_fee::get_field_count() == 2);
    std::cout << "field offset0: " << type_info_traits_fee::get_field_offset<0>() << std::endl;
    std::cout << "field offset1: " << type_info_traits_fee::get_field_offset<1>() << std::endl;
    static_assert(std::is_same_v<decltype(type_info_traits_fee::get_field_type<0>()), double>);
    static_assert(std::is_same_v<decltype(type_info_traits_fee::get_field_type<1>()), int>);

    using type_info_traits_ta = punk::type_info_traits<test_align>;
    static_assert(type_info_traits_ta::get_field_count() == 4);
    std::cout << "field offset0: " << type_info_traits_ta::get_field_offset<0>() << std::endl;
    std::cout << "field offset1: " << type_info_traits_ta::get_field_offset<1>() << std::endl;
    std::cout << "field offset2: " << type_info_traits_ta::get_field_offset<2>() << std::endl;
    std::cout << "field offset3: " << type_info_traits_ta::get_field_offset<3>() << std::endl;
    static_assert(std::is_same_v<decltype(type_info_traits_ta::get_field_type<0>()), int>);
    static_assert(std::is_same_v<decltype(type_info_traits_ta::get_field_type<1>()), char>);
    static_assert(std::is_same_v<decltype(type_info_traits_ta::get_field_type<2>()), double>);
    static_assert(std::is_same_v<decltype(type_info_traits_ta::get_field_type<3>()), std::string>);

    auto* rtti = punk::runtime_type_system::create_instance();
    auto* test_align_type_info = rtti->get_or_create_type_info<test_align>();
    std::cout << "type name:" << test_align_type_info->name << std::endl;
    std::cout << "type size:" << test_align_type_info->size << std::endl;
    std::cout << "type alignment:" << test_align_type_info->alignment << std::endl;
    std::cout << "type field count:" << test_align_type_info->fields.size() << std::endl;
    for (auto loop = 0; loop < test_align_type_info->fields.size(); ++loop)
    {
        auto const& field = test_align_type_info->fields[loop];
        std::cout << "field type name:" << field.type->name << std::endl;
        std::cout << "field offset:" << field.offset << std::endl;
    }

    auto* fee_type_info = syncAwait(rtti->async_get_or_create_type_info<fee>());
    std::cout << "type name:" << fee_type_info->name << std::endl;
    std::cout << "type size:" << fee_type_info->size << std::endl;
    std::cout << "type alignment:" << fee_type_info->alignment << std::endl;
    std::cout << "type field count:" << fee_type_info->fields.size() << std::endl;
    for (auto loop = 0; loop < fee_type_info->fields.size(); ++loop)
    {
        auto const& field = fee_type_info->fields[loop];
        std::cout << "field type name:" << field.type->name << std::endl;
        std::cout << "field offset:" << field.offset << std::endl;
    }

    return 0;
}