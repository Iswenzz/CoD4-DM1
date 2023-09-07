add_library(CoD4DM1.Lib)
set_target_properties(CoD4DM1.Lib PROPERTIES OUTPUT_NAME CoD4DM1)

add_subdirectory(API)
add_subdirectory(Crypt)
add_subdirectory(Demo)
add_subdirectory(Utils)

target_link_libraries(CoD4DM1.Lib PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(CoD4DM1.Lib PUBLIC .)

get_target_property(SOURCES CoD4DM1.Lib SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
