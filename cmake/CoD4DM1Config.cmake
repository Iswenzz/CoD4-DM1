@PACKAGE_INIT@
include(CMakeFindDependencyMacro)

set(CoD4DM1_INCLUDE_DIRS "@CMAKE_INSTALL_PREFIX@/include")
find_dependency(nlohmann_json CONFIG REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/CoD4DM1Targets.cmake")
