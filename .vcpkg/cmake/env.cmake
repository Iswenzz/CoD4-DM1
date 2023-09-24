if(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "ERROR: VCPKG_ROOT environment variable is not defined. Please set it to the root directory of your vcpkg installation.")
endif()

file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOT)
