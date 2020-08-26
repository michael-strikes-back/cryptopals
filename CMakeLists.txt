cmake_minimum_required (VERSION 3.5)

project (cryptopals)

include_directories (include)

if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14")
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")
set (source_dir "${PROJECT_SOURCE_DIR}/src/")
set (lib_dir "${PROJECT_SOURCE_DIR}/libs/")

file (GLOB source_files "${source_dir}/*.cpp")
file (GLOB object_files "${lib_dir}/*.o")

ADD_LIBRARY(LibsModule ${source_files})
target_link_libraries (LibsModule -lssl)
target_link_libraries (LibsModule -lcrypto)

add_executable (cryptopals ${object_files})
target_link_libraries (cryptopals LibsModule)

