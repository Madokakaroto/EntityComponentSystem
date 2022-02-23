#include <cstdint>
#include <concepts>
#include <limits>
#include <compare>
#include <iostream>
#include "boost/pfr.hpp"
#include "Forward.hpp"
#include "Hash.hpp"
#include "Handle.hpp"
#include "Entity.h"

int main(void)
{
	constexpr ecs::entity_t e;
	static_assert(!e.is_valid());

	std::cout << "hello ecs" << std::endl;

	static_assert(ecs::static_murmur_hash_x86_32("hello ecs", 0x453627) != 0);

	return 0;
}