cmake_minimum_required(VERSION 3.5)

project(PunkEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()

set(PK_SOURCE
    src/Meta.cpp
	main.cpp)

set(DEPENDENCIES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

include_directories(
	${DEPENDENCIES_DIRECTORY}/boost/pfr/include
    ${DEPENDENCIES_DIRECTORY}/boost/preprocessor/include
    ${DEPENDENCIES_DIRECTORY}/asio/asio/include
    ${DEPENDENCIES_DIRECTORY}/async_simple
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(PunkEngine ${PK_SOURCE})

link_libraries(Executors -lc++ -lstdc++)