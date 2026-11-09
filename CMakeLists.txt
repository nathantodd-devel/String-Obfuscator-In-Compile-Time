cmake_minimum_required(VERSION 3.8)
project(StringObfuscator CXX)

# Header-only library. Consumers just link against it to get the include path.
add_library(obfuscator INTERFACE)
target_include_directories(obfuscator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Pick the standard with -DCMAKE_CXX_STANDARD=11|14|17|20|23|26 (default 17).
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example example/main.cpp)
target_link_libraries(example PRIVATE obfuscator)
