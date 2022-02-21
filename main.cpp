#include <cstdint>
#include <concepts>
#include <limits>
#include <compare>
#include <iostream>
#include "Handle.hpp"
#include "Entity.h"

int main(void)
{
	constexpr ecs::entity_t e;
	static_assert(!e.is_valid());

	std::cout << "hello ecs" << std::endl;
	return 0;
}