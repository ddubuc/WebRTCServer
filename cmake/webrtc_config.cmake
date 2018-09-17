cmake_minimum_required(VERSION 3.7.1)

if (WIN32)
  SET(WEBRTC_ROOT "${HIPE_EXTERNAL_DIR}/webrtc")
else()
  SET(WEBRTC_ROOT "${HIPE_EXTERNAL_DIR}/")
endif()
