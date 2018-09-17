cmake_minimum_required(VERSION 3.7.1)

set(COMPONENTS yuv)

macro(_yuv_ADJUST_LIB_VARS basename)
  if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    set(yuv_${basename}_LIBRARY optimized ${yuv_${basename}_LIBRARY_RELEASE} debug ${yuv_${basename}_LIBRARY_DEBUG})
  else(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    # if there are no configuration types and CMAKE_BUILD_TYPE has no value
    # then just use the release libraries
    set(yuv_${basename}_LIBRARY ${yuv_${basename}_LIBRARY_DEBUG} )
  endif(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
  # FIXME: This probably should be set for both cases
  set(yuv_${basename}_LIBRARIES optimized ${yuv_${basename}_LIBRARY_RELEASE} debug ${yuv_${basename}_LIBRARY_DEBUG})
endmacro(_yuv_ADJUST_LIB_VARS basename)

set(Libyuv_DIR "yuv-ROOT-NOTFOUND" CACHE PATH "Path to the root yuv directory" )
set(Libyuv_LIBRARYDIR "yuv-LIBRARY-NOTFOUND" CACHE PATH "Path to the static yuv Library" )


if(${Libyuv_DIR} EQUAL "yuv-ROOT-NOTFOUND")
  message( FATAL_ERROR "Variable Libyuv_DIR is empty")
endif(${Libyuv_DIR} EQUAL "yuv-ROOT-NOTFOUND")

set(Libyuv_INCLUDEDIR "${Libyuv_DIR}/include;" CACHE PATH "include path for yuv" FORCE)

if(WIN32)
  set(Libyuv_LIBRARYDIR "${Libyuv_DIR}/lib" CACHE PATH "include path for yuv" FORCE)
endif(WIN32)

if(UNIX)
  set(Libyuv_LIBRARYDIR "${Libyuv_DIR}/lib" CACHE PATH "include path for yuv" FORCE)
endif(UNIX)



set(lib_path ${Libyuv_LIBRARYDIR})
set(_lib_list "")

# FIXME
# Centralize this to avoid redundancy.
if(WIN32)
  set(EXTENSION .lib)
else(WIN32)
  set(EXTENSION .a)
endif(WIN32)

foreach(COMPONENT  ${COMPONENTS})
  string( TOUPPER ${COMPONENT} UPPERCOMPONENT )
  if(NOT TARGET yuv::${COMPONENT})
    string( TOUPPER ${COMPONENT} UPPERCOMPONENT )
    set(yuv_${UPPERCOMPONENT}_LIBRARY "")
    set(yuv_${UPPERCOMPONENT}_LIBRARY_RELEASE "")
    set(yuv_${UPPERCOMPONENT}_LIBRARY_DEBUG   "")
  endif(NOT TARGET yuv::${COMPONENT})
endforeach(COMPONENT  ${COMPONENTS})

foreach(COMPONENT  ${COMPONENTS})
  if(NOT TARGET yuv::${COMPONENT})
    string(TOUPPER ${COMPONENT} UPPERCOMPONENT )
    message(STATUS "Set LIBRARY : ${COMPONENT}")
    set(yuv_${UPPERCOMPONENT}_LIBRARY "")
    if(WIN32)
      set(yuv_${UPPERCOMPONENT}_LIBRARY_RELEASE "${lib_path}/${COMPONENT}${EXTENSION}")
      set(yuv_${UPPERCOMPONENT}_LIBRARY_DEBUG   "${lib_path}/${COMPONENT}d${EXTENSION}")
    endif(WIN32)

    if(UNIX)
      set(yuv_${UPPERCOMPONENT}_LIBRARY_RELEASE "${lib_path}/lib${COMPONENT}${EXTENSION}")
      set(yuv_${UPPERCOMPONENT}_LIBRARY_DEBUG   "${lib_path}/Debug/lib${COMPONENT}${EXTENSION}")
    endif(UNIX)

    _yuv_ADJUST_LIB_VARS(${UPPERCOMPONENT})
    add_library(yuv::${COMPONENT} SHARED IMPORTED)
    message(STATUS "Select LIBRARY : ${yuv_${UPPERCOMPONENT}_LIBRARY}")

    if(EXISTS "${yuv_${UPPERCOMPONENT}_LIBRARY}")
      set_target_properties(yuv::${COMPONENT} PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${yuv_${UPPERCOMPONENT}_LIBRARY}")
    endif(EXISTS "${yuv_${UPPERCOMPONENT}_LIBRARY}")

    if(EXISTS "${yuv_${UPPERCOMPONENT}_LIBRARY_RELEASE}")
      set_property(TARGET yuv::${COMPONENT} APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(yuv::${COMPONENT} PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
      IMPORTED_LOCATION_RELEASE "${yuv_${UPPERCOMPONENT}_LIBRARY_RELEASE}")
    endif(EXISTS "${yuv_${UPPERCOMPONENT}_LIBRARY_RELEASE}")

    if(EXISTS "${yuv_${UPPERCOMPONENT}_LIBRARY_DEBUG}")
      set_property(TARGET yuv::${COMPONENT} APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(yuv::${COMPONENT} PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
      IMPORTED_LOCATION_DEBUG "${yuv_${UPPERCOMPONENT}_LIBRARY_DEBUG}")
    endif(EXISTS "${yuv_${UPPERCOMPONENT}_LIBRARY_DEBUG}")

  endif(NOT TARGET yuv::${COMPONENT})
endforeach(COMPONENT  ${COMPONENTS})

set(Libyuv_LIBRARIES ""  CACHE LIST "yuvMedia libraries " FORCE)
  foreach(COMPONENT  ${COMPONENTS})
    string( TOUPPER ${COMPONENT} UPPERCOMPONENT )
    list(APPEND Libyuv_LIBRARIES ${yuv_${UPPERCOMPONENT}_LIBRARY})
  endforeach(COMPONENT  ${COMPONENTS})
  ##Append last dll comming from vendor and zlib
list(APPEND Libyuv_LIBRARIES ${yuv_EXTERNAL_LIBRARY})

message(STATUS "Libyuv LIBRARIES : ${Libyuv_LIBRARIES}")

