cmake_minimum_required(VERSION 3.0.0)
project(cod4_dm1 VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(cod4_dm1
	main.cpp
	src/Demo.cpp
	src/Huffman.cpp
	src/Msg.cpp
	src/NetFields.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-fpermissive -Wno-narrowing")