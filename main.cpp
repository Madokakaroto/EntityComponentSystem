
#include "Forward.hpp"

#include "Types.hpp"
#include "Common.hpp"


struct foo {};

class fee {};

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

	return 0;
}