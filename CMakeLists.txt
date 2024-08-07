cmake_minimum_required(VERSION 3.27)
project(CoD4DM1 VERSION 1.0.6 LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# CMake
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ClangTools)

# CPack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR 7Z)
include(CPack)

# Dependencies
find_package(nlohmann_json CONFIG REQUIRED)

# Tests
if(BUILD_TESTING)
	include(CTest)
	find_package(GTest CONFIG REQUIRED)

	if(MSVC)
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()

	add_executable(CoD4DM1.Tests)

	target_link_libraries(CoD4DM1.Tests PRIVATE
		CoD4DM1
		GTest::gtest
		GTest::gtest_main
		GTest::gmock
		GTest::gmock_main)

	add_test(NAME Tests COMMAND CoD4DM1.Tests)
endif()

# Build
add_subdirectory(fixtures)
add_subdirectory(src)

if(BUILD_CLI)
	find_package(cxxopts CONFIG REQUIRED)
	add_executable(CoD4DM1.CLI src/CLI.cpp)
	set_target_properties(CoD4DM1.CLI PROPERTIES OUTPUT_NAME CoD4DM1)
	target_link_libraries(CoD4DM1.CLI PRIVATE CoD4DM1 cxxopts::cxxopts)
	install(TARGETS CoD4DM1.CLI DESTINATION bin)
endif()

# Install
install(TARGETS CoD4DM1 EXPORT CoD4DM1Targets LIBRARY DESTINATION lib)
install(DIRECTORY src/ DESTINATION include/CoD4DM1 FILES_MATCHING PATTERN "*.hpp" PATTERN "__test__" EXCLUDE)

# Package
include(CMakePackageConfigHelpers)

export(EXPORT CoD4DM1Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/CoD4DM1Targets.cmake"
    NAMESPACE CoD4DM1::)

configure_package_config_file(cmake/CoD4DM1Config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/CoD4DM1Config.cmake"
	INSTALL_DESTINATION lib/cmake/CoD4DM1)

install(EXPORT CoD4DM1Targets
    FILE CoD4DM1Targets.cmake
    NAMESPACE CoD4DM1::
    DESTINATION lib/cmake/CoD4DM1)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/CoD4DM1Config.cmake"
    DESTINATION lib/cmake/CoD4DM1)
