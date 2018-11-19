cmake_minimum_required(VERSION 3.7.1)

# include_directories(.)

find_package(Threads REQUIRED)

set(BOOST_COMPONENTS system thread filesystem regex)

# Late 2017 TODO: remove the following checks and always use std::regex
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    list(APPEND BOOST_COMPONENTS regex)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BOOST_REGEX")
  endif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

#add_definitions(-DBOOST_ALL_NO_LIB)
#add_definitions(-DBOOST_ALL_DYN_LINK)
if(WIN32)
  add_definitions(-DBOOST_USE_WINAPI_VERSION=0x601)
  set(Boost_DEBUG OFF)
  set(Boost_USE_STATIC_LIBS ON)# only find static libs
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)
else()
  set(Boost_DEBUG OFF)
  set(Boost_USE_STATIC_LIBS OFF)# only find static libs
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)
endif(WIN32)

if (WIN32)
  find_package(Boost 1.66.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
else()
  find_package(Boost 1.66.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
endif()
if(NOT ${Boost_FOUND})
  message(FATAL_ERROR "failed to find Boost library (compatible with version 1.62.0)")
endif(NOT ${Boost_FOUND})

display_pathlist("Boost_INCLUDE_DIR" "${Boost_INCLUDE_DIR}")
display_pathlist("Boost_LIBRARIES" "${Boost_LIBRARIES}")
#prepend_include_directories_if_necessary("${Boost_INCLUDE_DIR}")
