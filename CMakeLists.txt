cmake_minimum_required(VERSION 3.25)

project(libe131 C)

set (SOURCE_FILES src/e131.c)
set (HEADER_FILES src/e131.h)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
  PRIVATE ${SOURCE_FILES} 
  PUBLIC FILE_SET HEADERS
  BASE_DIRS src
  FILES ${HEADER_FILES} 
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
  $<INSTALL_INTERFACE:src>
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  FILE_SET HEADERS
  DESTINATION include
)

install(
  EXPORT ${PROJECT_NAME} 
  NAMESPACE libe131:: 
  DESTINATION share
)
