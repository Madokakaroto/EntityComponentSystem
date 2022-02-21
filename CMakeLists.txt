cmake_minimum_required(VERSION 3.0)

project(ECS)

set(CMAKE_CXX_STANDARD 20)

set(ECS_SOURCES
	main.cpp)

add_executable(ECS ${ECS_SOURCES})

link_libraries(Executors -lc++ -lstdc++)