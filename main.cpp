
#include "Forward.hpp"

#include "Common.hpp"

#include "Handle.hpp"
#include "Entity.hpp"
#include "Types.hpp"

struct foo {};

class fee {};

int main(void)
{
	constexpr ecs::entity_t e{};
	static_assert(!e.is_valid());

	std::cout << "hello ecs" << std::endl;

	static_assert(ecs::static_murmur_hash_x86_32("hello ecs", 0x453627) != 0);

	std::cout << ecs::demangle_name<int>() << std::endl;
	std::cout << ecs::demangle_name<std::string>() << std::endl;
	std::cout << ecs::demangle_name<foo>() << std::endl;
	std::cout << ecs::demangle_name<fee>() << std::endl;

	return 0;
}