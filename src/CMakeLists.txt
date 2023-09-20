add_library(CoD4DM1)

add_subdirectory(API)
add_subdirectory(Crypt)
add_subdirectory(Demo)
add_subdirectory(Utils)

target_link_libraries(CoD4DM1 PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(CoD4DM1 PRIVATE .)

target_include_directories(CoD4DM1 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/CoD4DM1>)

get_target_property(SOURCES CoD4DM1 SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
