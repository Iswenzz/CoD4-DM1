cmake_minimum_required(VERSION 3.16.0)
project(CoD4DM1 VERSION 0.1.0 LANGUAGES CXX)

# CPack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR 7Z)
include(CPack)

# Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Tests
# include(CTest)
# add_executable(CoD4DM1Tests src/tests/main.test.cpp)
# target_link_libraries(CoD4DM1Tests PUBLIC ${CONAN_LIBS} CoD4DM1Lib)
# add_test(NAME Tests COMMAND CoD4DM1Tests)

# Build
add_subdirectory(src)
add_executable(CoD4DM1 src/CLI.cpp)
target_link_libraries(CoD4DM1 PRIVATE ${CONAN_LIBS} CoD4DM1Lib)
install(TARGETS CoD4DM1 DESTINATION bin)
install(TARGETS CoD4DM1Lib LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION lib/include)
