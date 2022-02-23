cmake_minimum_required(VERSION 3.0)

project(ECS)

set(CMAKE_CXX_STANDARD 20)

set(ECS_SOURCES
	main.cpp)

set(DEPENDENCIES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

include_directories(
	${DEPENDENCIES_DIRECTORY}/pfr/include
)

add_executable(ECS ${ECS_SOURCES})

link_libraries(Executors -lc++ -lstdc++)