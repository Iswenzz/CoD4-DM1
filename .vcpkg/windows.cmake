include(.vcpkg/cmake/env.cmake)
set(VCPKG_TARGET_TRIPLET x64-windows-static)

option(BUILD_CLI "Build CLI" OFF)
if(BUILD_CLI)
	list(APPEND VCPKG_MANIFEST_FEATURES "cli")
endif()

option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
	list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
