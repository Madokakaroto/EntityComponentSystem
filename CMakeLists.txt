cmake_minimum_required(VERSION 3.5)

project(PunkEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()

# project folder tree
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE PUBLIC_INCLUDE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE PRIVATE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set(PUNK_MAIN "main.cpp")

assign_source_group(${PRIVATE_FILES})
assign_source_group(${PUBLIC_INCLUDE_FILES})
assign_source_group(${PUNK_MAIN})

set(DEPENDENCIES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

include_directories(
	${DEPENDENCIES_DIRECTORY}/boost/pfr/include
    ${DEPENDENCIES_DIRECTORY}/boost/preprocessor/include
    ${DEPENDENCIES_DIRECTORY}/asio/asio/include
    ${DEPENDENCIES_DIRECTORY}/async_simple
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(PunkEngine ${PRIVATE_FILES} ${PUBLIC_INCLUDE_FILES} ${PUNK_MAIN})
link_libraries(PunkEngine -lc++ -lstdc++)