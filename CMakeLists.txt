cmake_minimum_required (VERSION 3.5)

project (cryptopals)

include_directories (include)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14")
set (source_dir "${PROJECT_SOURCE_DIR}/src/")
set (lib_dir "${PROJECT_SOURCE_DIR}/libs/")

file (GLOB source_files "${source_dir}/*.cpp")
file (GLOB object_files "${lib_dir}/*.o")

#target_link_libraries (cryptopals ${LINK_OPTIONS})
add_executable (cryptopals ${source_files} ${object_files})

