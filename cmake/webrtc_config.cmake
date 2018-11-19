cmake_minimum_required(VERSION 3.7.1)

if (WIN32)
  SET(WEBRTC_ROOT "${HIPE_EXTERNAL_DIR}/webrtc")
else()
list(APPEND WEBRTC_INCLUDE_DIRS ${WEBRTC_ROOT}/include)
list(APPEND WEBRTC_INCLUDE_DIRS ${WEBRTC_ROOT}/include/third_party/boringssl/src/include/)
list(APPEND WEBRTC_INCLUDE_DIRS ${WEBRTC_ROOT}/include/third_party/openssl/src/include/)
list(APPEND WEBRTC_INCLUDE_DIRS ${WEBRTC_ROOT}/include/third_party/)


list(APPEND WEBRTC_LINK_DIRECTORIES "optimized")
list(APPEND WEBRTC_LINK_DIRECTORIES "${WEBRTC_ROOT}/lib/x64/Release")
list(APPEND WEBRTC_LINK_DIRECTORIES "debug")
list(APPEND WEBRTC_LINK_DIRECTORIES "${WEBRTC_ROOT}/lib/x64/Debug")
link_directories(${WEBRTC_LINK_DIRECTORIES})
SET(WEBRTC_LIBRARIES libwebrtc_full.a CACHE FILEPATH "WEBRTC_LIBRARY")
endif()
