option(${PROJECT_NAME}_ENABLE_DOXYGEN ON)
option(BUILD_DOC "Build documentation" ON)

option(${PROJECT_NAME}_WARNINGS_AS_ERRORS ON)

option(${PROJECT_NAME}_CLANG_TIDY ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(${PROJECT_NAME}_CCACHE ON)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()
message("-- Config applied")
