cmake_minimum_required (VERSION 3.5)

project (cryptopals VERSION 0.7)

include_directories (include)

if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14")
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")
set (source_dir "${PROJECT_SOURCE_DIR}/src/")

file (GLOB source_files "${source_dir}/*.cpp")

add_executable (cryptopals ${source_files})
target_link_libraries (cryptopals -lssl)
target_link_libraries (cryptopals -lcrypto)

