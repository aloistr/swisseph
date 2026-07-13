cmake_minimum_required(VERSION 3.16)

project(swisseph
  VERSION 2.10.3
  LANGUAGES C
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(SWISSEPH_IS_TOP_LEVEL TRUE)
else()
  set(SWISSEPH_IS_TOP_LEVEL FALSE)
endif()

option(SWISSEPH_BUILD_PROGRAMS "Build the swetest/swevents/swemini/obama sample programs" ${SWISSEPH_IS_TOP_LEVEL})

set(SWISSEPH_SOURCES
  swedate.c
  swehouse.c
  swejpl.c
  swemmoon.c
  swemplan.c
  sweph.c
  swephlib.c
  swecl.c
  swehel.c
)

add_library(swisseph ${SWISSEPH_SOURCES})
add_library(swisseph::swisseph ALIAS swisseph)

set_target_properties(swisseph PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

target_include_directories(swisseph PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if(UNIX)
  target_link_libraries(swisseph PUBLIC m)
endif()

if(SWISSEPH_BUILD_PROGRAMS)
  foreach(program swetest swevents swemini obama)
    add_executable(${program} ${program}.c)
    target_link_libraries(${program} PRIVATE swisseph::swisseph)
  endforeach()
endif()
