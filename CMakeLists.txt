cmake_minimum_required (VERSION 3.7.1)

project (WebRTCServer)

option(HIPE_EXTERNAL_OPENCV "Use OpenCV libraries in Hipe External." ON)
option(HIPE_EXTERNAL_BOOST "Use Boost libraries in Hipe External." ON)
option(HIPE_EXTERNAL_LIBYUV "Use libyuv libraries in Hipe External." ON)

message(STATUS "HIPE_EXTERNAL_OPENCV: ${HIPE_EXTERNAL_OPENCV}")

# These settings are only required at the globel scope but they are collected
# here to enable inclusion in a nested stand-alone scope, e.g. for the
# compilation of a single HIPE library.


if (UNIX)
	include_directories("${CMAKE_SOURCE_DIR}/header" "/usr/include" "/usr/local/include")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unknown-pragmas")
else(WIN32)
	include_directories("${CMAKE_SOURCE_DIR}/header")
endif()

message(STATUS "Change path to install directory [${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT}]")
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install/WebRTCserver" CACHE PATH "Installation directory" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Check if HIPE_EXTERNAL has been set via the command-line via
#
# -DHIPE_EXTERNAL=<path>
#
# If not, check if the environment variable of the same name has been set and
# use that instead. If neither has been set, raise a fatal error.

# Unset via command-line.
if(NOT DEFINED HIPE_EXTERNAL)
	# Set via environment variable.
	if(DEFINED ENV{HIPE_EXTERNAL})
		set(HIPE_EXTERNAL "$ENV{HIPE_EXTERNAL}")
		# Unset via environment variable.
	else(DEFINED ENV{HIPE_EXTERNAL})
		set(HIPE_EXTERNAL "${CMAKE_SOURCE_DIR}/../3rdParty/HipeExternal")
		message(FATAL_ERROR "Could not find \"HIPE_EXTERNAL\". Please set it locally or by environment variable.")

	endif(DEFINED ENV{HIPE_EXTERNAL})
endif(NOT DEFINED HIPE_EXTERNAL)


# Inform the user which value was used.
message(STATUS "HIPE_EXTERNAL : ${HIPE_EXTERNAL}")
if(UNIX)

	if(NOT EXISTS "${HIPE_EXTERNAL}")
		file(MAKE_DIRECTORY "${HIPE_EXTERNAL}")
		file(MAKE_DIRECTORY "${HIPE_EXTERNAL}/linux64/install")
	endif()
else()


	if (NOT (${USE_PREBUILT_DEPENDENCIES} OR ${FORCE_USE_PREBUILT_DEPENDENCIES}) )
		# Make sure HIPE_EXTERNAL exists.
		if(NOT EXISTS "${HIPE_EXTERNAL}")
			message(FATAL_ERROR "${HIPE_EXTERNAL} does not exist")
		elseif(NOT IS_DIRECTORY "${HIPE_EXTERNAL}")
			message(FATAL_ERROR "${HIPE_EXTERNAL} is not a directory")
		endif(NOT EXISTS "${HIPE_EXTERNAL}")
	endif()
endif()

# Make sure that it is an absolute path.
get_filename_component(HIPE_EXTERNAL "${HIPE_EXTERNAL}" ABSOLUTE)



if(WIN32)
	set(HIPE_EXTERNAL_DIR "${HIPE_EXTERNAL}/win64")
else(WIN32)
	set(HIPE_EXTERNAL_DIR "${HIPE_EXTERNAL}/linux64/install")
	# Without this, the linker fails to find e.g. libboost_wave when building
	# multiple libraries ("cannot find -lboost_wave"), despite correctly locating
	# the libraries in HIPE_EXTERNAL with the find_package command. For example,
	# building streaming_rtsp or core separately succeeds but building both at the
	# same time results in the aforementioned error.
	#   link_libraries("-L '${HIPE_EXTERNAL_DIR}/lib'")
endif(WIN32)


if(WIN32)

	set(Boost_DIR "${HIPE_EXTERNAL_DIR}/boost_1_62_0/" CACHE PATH "Boost_DIR" FORCE)
	set(Boost_INCLUDE_DIR "${Boost_DIR}" CACHE PATH "Boost_INCLUDE_DIR" FORCE)
	set(BOOST_LIBRARYDIR "${Boost_DIR}/lib64-msvc-14.0" CACHE PATH "BOOST_LIBRARYDIR" FORCE)
	set(BOOST_ROOT "${Boost_DIR}/"  CACHE PATH "BOOST_ROOT" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${Boost_DIR}")


	set(OpenCV_DIR "${HIPE_EXTERNAL_DIR}/opencv-3.4" CACHE PATH "OpenCV Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${OpenCV_DIR}")

	set(Libyuv_DIR "${HIPE_EXTERNAL_DIR}/libyuv" CACHE PATH "Libyuv Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${Libyuv_DIR}")

	set(Websocketpp_DIR "${HIPE_EXTERNAL_DIR}/websocketpp" CACHE PATH "Websocketpp Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${Websocketpp_DIR}")

	set(WebRTC_DIR "${HIPE_EXTERNAL_DIR}/WebRTC" CACHE PATH "WebRTC Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${WebRTC_DIR}")

	set(JsonPP_DIR "${HIPE_EXTERNAL_DIR}/json" CACHE PATH "Websocketpp Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${JsonPP_DIR}")

	set(BoringSSL_DIR "${HIPE_EXTERNAL_DIR}/boringssl-webRTC" CACHE PATH "BOringSSL Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${BoringSSL_DIR}")

else()

	if(HIPE_EXTERNAL_BOOST)
		list(APPEND CMAKE_PREFIX_PATH "${HIPE_EXTERNAL_DIR}/boost")
	endif(HIPE_EXTERNAL_BOOST)

	if(HIPE_EXTERNAL_OPENCV)
		list(APPEND CMAKE_PREFIX_PATH "${HIPE_EXTERNAL_DIR}/cuda8")
		set(ENV{PATH} "${HIPE_EXTERNAL_DIR}/cuda8/bin:$ENV{PATH}")
		list(APPEND CMAKE_PREFIX_PATH "${HIPE_EXTERNAL_DIR}/opencv")
	endif(HIPE_EXTERNAL_OPENCV)


	if(HIPE_EXTERNAL_LIBYUV)
		set(Libyuv_DIR "${HIPE_EXTERNAL_DIR}" CACHE PATH "Libyuv Directory" FORCE)
		list(APPEND CMAKE_PREFIX_PATH "${Libyuv_DIR}")
	endif()

	list(APPEND CMAKE_PREFIX_PATH "${HIPE_EXTERNAL_DIR}/")

	set(Websocketpp_DIR "${HIPE_EXTERNAL_DIR}/" CACHE PATH "Websocketpp Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${Websocketpp_DIR}")

	set(WebRTC_DIR "${HIPE_EXTERNAL_DIR}" CACHE PATH "WebRTC Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${WebRTC_DI}")
	
	set(JsonPP_DIR "${HIPE_EXTERNAL_DIR}/" CACHE PATH "JsonPP Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${JsonPP_DIR}")

	set(BoringSSL_DIR "${HIPE_EXTERNAL_DIR}/boringssl-webRTC" CACHE PATH "BOringSSL Directory" FORCE)
	list(APPEND CMAKE_PREFIX_PATH "${BoringSSL_DIR}")

endif()


if (WIN32)

	set(CompilerFlags
		CMAKE_CXX_FLAGS
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_C_FLAGS
		CMAKE_C_FLAGS_DEBUG
		CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_RELWITHDEBINFO
	)
	foreach(CompilerFlag ${CompilerFlags})
		string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()

	option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)

	set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
	include(InstallRequiredSystemLibraries)

endif(WIN32)


include("${CMAKE_SOURCE_DIR}/cmake/common_functions_and_macros.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/boost_config.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/webrtc_config.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/opencv_config.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/libyuv_config.cmake")

message(STATUS "Get WebRTC from ${WEBRTC_ROOT}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(LibWebRTC
	REQUIRED
	PATHS "${WEBRTC_ROOT}/lib/cmake/LibWebRTC"
NO_DEFAULT_PATH)

find_package(jsoncpp
	REQUIRED
	PATHS "${JsonPP_DIR}/lib/cmake/jsoncpp"
NO_DEFAULT_PATH)


#controls where to build the executables after "make"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target/lib)
set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target/include)

#Add some multithreaded build support =====================================================================================================
if(WIN32)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	add_compile_options(/wd4820 /wd4668 /wd4619 /wd4626 /wd5031 /wd4365 /wd4710 /wd4514 /wd4503)
  else()
	add_compile_options(-fPIC)

endif()




add_lib(WebRTCServer "${CMAKE_SOURCE_DIR}/source" SHARED)

if (WIN32)
  target_compile_definitions(WebRTCServer PRIVATE -DNOMINMAX -DWEBRTC_WIN)
else()
  target_compile_definitions(WebRTCServer PRIVATE -DWEBRTC_POSIX)
endif()

target_compile_definitions(WebRTCServer PRIVATE -DWEBRTCSERVER_BUILD)

include_directories(WebRTCServer ${WEBRTC_INCLUDE_DIRS} )
target_include_directories(WebRTCServer PRIVATE ${WEBRTC_INCLUDE_DIRS})
target_include_directories(WebRTCServer PRIVATE ${HIPE_EXTERNAL_DIR})
target_include_directories(WebRTCServer PRIVATE ${Boost_INCLUDE_DIRS})

target_include_directories(WebRTCServer PRIVATE ${BoringSSL_DIR})
target_include_directories(WebRTCServer PRIVATE ${Libyuv_INCLUDEDIR})

target_link_libraries(WebRTCServer webrtc)
if (WIN32)
  target_link_libraries(WebRTCServer jsoncpp_lib)
else()
  target_link_libraries(WebRTCServer jsoncpp_lib_static)
endif()

target_link_libraries(WebRTCServer ${Libyuv_LIBRARIES})
target_link_libraries(WebRTCServer ${OpenCV_LIBS})
target_link_libraries(WebRTCServer ${Boost_LIBRARIES})
target_link_libraries(WebRTCServer ${CMAKE_THREAD_LIBS_INIT})



if(WIN32) 
	message(STATUS "Test if it works with windows")
		target_link_libraries(WebRTCServer ws2_32 wsock32 winmm dmoguids Secur32 msdmo wmcodecdspuuid amstrmid crypt32 iphlpapi secur32 Strmiids)

	endif(WIN32)
	SET_TARGET_PROPERTIES( WebRTCServer PROPERTIES OUTPUT_NAME WebRTCServer
	DEBUG_OUTPUT_NAME WebRTCServerd )
	#install_header_to_target(WebRTCServer)
	message(STATUS "Include build folder : ${CMAKE_CURRENT_SOURCE_DIR}/source/header/")
	
	TARGET_INCLUDE_DIRECTORIES( WebRTCServer PUBLIC
                                   $<INSTALL_INTERFACE:include>
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source/header/>)
	
	set_target_properties(WebRTCServer PROPERTIES
	  MAP_IMPORTED_CONFIG_MINSIZEREL Release
	  MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
	  )
	set_target_properties(WebRTCServer PROPERTIES INTERFACE_LINK_LIBRARIES "")
	set_target_properties(WebRTCServer PROPERTIES IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG "")
	
	install(DIRECTORY "${CMAKE_SOURCE_DIR}/source/header/" DESTINATION include
			COMPONENT sources
			PATTERN "*.h" 
			PATTERN "internal" EXCLUDE
			PATTERN "build" EXCLUDE
			PATTERN "install" EXCLUDE)
			
	install (TARGETS WebRTCServer
		EXPORT WebRTCServer
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		COMPONENT library
		ARCHIVE DESTINATION lib
	)


	install(EXPORT WebRTCServer DESTINATION lib/cmake COMPONENT library FILE WebRTCServerConfig.cmake)
	# install(EXPORT WebRTCServerDebug   DESTINATION "lib/cmake" COMPONENT library)
	# install(EXPORT WebRTCServerRelWithDebugInfo   DESTINATION "lib/cmake" COMPONENT library)
