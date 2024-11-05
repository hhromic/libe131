cmake_minimum_required(VERSION 3.13)
project(libe131 C)

OPTION( SKIP_EXAMPLES "Skip Examples" OFF )

set (SOURCE_FILES 
  src/e131.c
  src/e131.h
)

add_library(${PROJECT_NAME} STATIC
        ${SOURCE_FILES} 
)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

if (NOT SKIP_EXAMPLES)
  add_subdirectory(examples)
endif()

#####################################
# Install

install(DIRECTORY src/ DESTINATION include/ FILES_MATCHING PATTERN *.h)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

set(config_in_builddir ${CMAKE_BINARY_DIR}/share/libe131/cmake/libe131-config.cmake)
configure_file(
  libe131-config.cmake.in
  ${config_in_builddir}
  COPYONLY
)
install(
  FILES ${config_in_builddir}
  DESTINATION share/libe131/cmake
)

set(targets_in_builddir ${CMAKE_BINARY_DIR}/share/libe131/cmake/libe131-targets.cmake)
export(
  EXPORT ${PROJECT_NAME}
  FILE ${targets_in_builddir}
)
install(
  EXPORT ${PROJECT_NAME}
  DESTINATION share/libe131/cmake
  FILE libe131-targets.cmake
)