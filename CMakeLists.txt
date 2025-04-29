cmake_minimum_required(VERSION 3.5)

project(ECS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()

set(ECS_SOURCES
    src/Meta.cpp
	main.cpp)

set(DEPENDENCIES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

include_directories(
	${DEPENDENCIES_DIRECTORY}/pfr/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(ECS ${ECS_SOURCES})

link_libraries(Executors -lc++ -lstdc++)