cmake_minimum_required(VERSION 3.16.0)
project(CoD4DM1 VERSION 1.0.5 LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CPack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR 7Z)
include(CPack)

# Dependencies
find_package(cxxopts CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Tests
if(BUILD_TESTING)
	include(CTest)
	find_package(GTest CONFIG REQUIRED)
	add_executable(CoD4DM1.Tests)

	target_link_libraries(CoD4DM1.Tests PRIVATE
		CoD4DM1.Lib
		GTest::gtest
		GTest::gtest_main
		GTest::gmock
		GTest::gmock_main)

	add_test(NAME Tests COMMAND CoD4DM1.Tests)
endif()

# Build
add_subdirectory(fixtures)
add_subdirectory(src)
add_executable(CoD4DM1 src/CLI.cpp)

target_link_libraries(CoD4DM1 PRIVATE CoD4DM1.Lib)

# Pack
install(TARGETS CoD4DM1 DESTINATION bin)
install(TARGETS CoD4DM1.Lib LIBRARY DESTINATION lib)
install(DIRECTORY src/ DESTINATION lib/include/CoD4DM1
    FILES_MATCHING PATTERN "*.hpp" PATTERN "__test__" EXCLUDE)
