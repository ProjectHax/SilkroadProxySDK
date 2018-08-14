cmake_minimum_required(VERSION 2.8)

project(example)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -fPIC -Wall")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../lib)

find_package(Boost 1.49 COMPONENTS system random thread chrono program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR} include)

add_library(SilkroadSecurity SHARED IMPORTED)

if (APPLE)
	set_property(TARGET SilkroadSecurity PROPERTY IMPORTED_LOCATION_RELEASE ../lib/libSilkroadSecurity.dylib)
else()
	set_property(TARGET SilkroadSecurity PROPERTY IMPORTED_LOCATION_RELEASE ../lib/libSilkroadSecurity.so)
endif()

add_library(example SHARED src/main.cpp src/base_plugin.cpp)

target_link_libraries(example ${Boost_LIBRARIES} pthread curl SilkroadSecurity)