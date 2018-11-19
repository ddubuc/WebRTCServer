


function(message_header header_text)
  string(REGEX REPLACE "." "#" _hash_spacer "${header_text}")
  message(
    STATUS
    "\n\
-- ######${_hash_spacer}######\n\
-- #     ${header_text}     #\n\
-- ######${_hash_spacer}######"
  )
endfunction(message_header)



function(display_pathlist _header _paths)
  set(_list_separator "\n--   ")
  string(REPLACE ";" "${_list_separator}" _path_list "${_paths}")
  message(STATUS "${_header}:${_list_separator}${_path_list}")
endfunction(display_pathlist _paths)

# Create source groups for visual studio.
function(group_for_visual_studio _subgroup _root_dir _file_list)
  foreach(_file_path IN ITEMS ${_file_list})
    message(STATUS "grouping for visual studio: ${_file_path}")
    # Get the directory. Replace PATH with DIRECTORY for CMake > 2.8.11
    get_filename_component(_dir_path "${_file_path}" PATH)
    file(RELATIVE_PATH _dir_path_rel "${_root_dir}" "${_dir_path}")
    string(REPLACE "/" "\\" _group_path "${_dir_path_rel}")
    source_group("${_subgroup}\\${_group_path}" FILES "${_file_path}")
  endforeach(_file_path IN ITEMS ${_file_list})
endfunction(group_for_visual_studio)

# Regroup all common commands in a macro for building HIPE libraries to avoid
# redundancy.

# TODO
# Refactor. The current version is directly inhereted and was only slightly
# modified to create the macro.

macro(add_lib _lib_name _src_root_path _lib_type)
  message_header("HIPE LIBRARY: ${_lib_name}")

  set(_src_lib_path ${_src_root_path}/src)
  set(_src_lib_inc_path ${_src_root_path}/header)

  message(STATUS "${_lib_name} source path: ${_src_lib_path}" )

  file(
    GLOB_RECURSE _source_list
    LIST_DIRECTORIES false
    "${_src_lib_path}/*.c*"
  )

  file(
    GLOB_RECURSE _header_list
    LIST_DIRECTORIES false
    "${_src_lib_inc_path}/*.h*"
  )

  # LIBRARY
#   if(HIPE_STATIC_LIBS)
#     set(_lib_type "STATIC")
#   else(HIPE_STATIC_LIBS)
#     set(_lib_type "SHARED")
#   endif(HIPE_STATIC_LIBS)

  add_library(${_lib_name} ${_lib_type} ${_source_list} ${_header_list})
  target_include_directories(${_lib_name} BEFORE PRIVATE ${_src_lib_inc_path})

  if (UNIX)
    set_property(TARGET ${_lib_name} PROPERTY POSITION_INDEPENDENT_CODE ON)
  #   target_compile_options(${_lib_name} PRIVATE -fPIC)
  endif(UNIX)


if (WIN32)
  # Group source files for Visual Studio
  group_for_visual_studio("source" "${_src_lib_path}" "${_source_list}")
  group_for_visual_studio("header" "${_src_lib_inc_path}" "${_header_list}")
endif(WIN32)

endmacro(add_lib)