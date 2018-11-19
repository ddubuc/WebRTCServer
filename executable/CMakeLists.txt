cmake_minimum_required (VERSION 3.7.1)

add_compile_options(-fno-rtti -std=c++11 -L/home/ddubuc/workspace/libwebrtc/out/webrtc/src/build/linux/debian_jessie_amd64-sysroot/usr/lib/x86_64-linux-gnu/)
add_compile_options(-L${HIPE_EXTERNAL_DIR}/lib)
message(STATUS "Add directory -L${CMAKE_SOURCE_DIR}/../../../install/lib/")
add_executable(testWebService main.cpp)

if (WIN32)
  target_compile_definitions(testWebService PRIVATE -DNOMINMAX -DWEBRTC_WIN )
else()
  target_compile_definitions(testWebService PRIVATE -DWEBRTC_POSIX) # -D_GLIBCXX_USE_CXX11_ABI=0) # -fno-rtti)
endif()

include_directories(testWebService ${CMAKE_SOURCE_DIR}/../source/header/)
include_directories(testWebService $ENV{HIPE_EXTERNAL}/opencv/include)

if (WIN32)
target_link_libraries(testWebService WebRTCServer)
else()
target_link_libraries(testWebService -std=c++11 WebRTCServer)
endif()

target_link_libraries(testWebService ${OpenCV_LIBS})
if (UNIX)
 target_link_libraries(testWebService -L${HIPE_EXTERNAL_DIR}/lib -pthread)
#target_link_libraries(testWebService -L${HIPE_EXTERNAL_DIR}/lib -lX11 -lXcomposite -lxcb -lXfixes -lXdamage ${CMAKE_SOURCE_DIR}/../../../install/lib/libssl.so ${CMAKE_SOURCE_DIR}/../../../install/lib/libcrypto.so /home/ddubuc/3rdParty/hipe_external/linux64/build/libwebrtc/build/webrtc/src/out/Release/obj/third_party/yasm/libyasm_utils.a -ldl libyasm.so -pthread)
endif()
