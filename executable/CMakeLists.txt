cmake_minimum_required (VERSION 3.7.1)


SET(HIPE_EXTERNAL_DIR "$ENV{HIPE_EXTERNAL}/linux64/install")
list(APPEND CMAKE_PREFIX_PATH "${HIPE_EXTERNAL_DIR}/cuda8")
set(ENV{PATH} "${HIPE_EXTERNAL_DIR}/cuda8/bin:$ENV{PATH}")
list(APPEND CMAKE_PREFIX_PATH "${HIPE_EXTERNAL_DIR}/opencv")
include("${CMAKE_SOURCE_DIR}/../cmake/opencv_config.cmake")
add_compile_options(-fno-rtti -std=c++11 -std=gnu++0x -L/home/ddubuc/workspace/libwebrtc/out/webrtc/src/build/linux/debian_jessie_amd64-sysroot/usr/lib/x86_64-linux-gnu/)
add_compile_options(-L${HIPE_EXTERNAL_DIR}/lib)
message(STATUS "Add directory -L${CMAKE_SOURCE_DIR}/../../../install/lib/")
add_executable(testWebService main.cpp)

target_compile_definitions(testWebService PRIVATE -DWEBRTC_POSIX -D_GLIBCXX_USE_CXX11_ABI=0)

include_directories(testWebService ${CMAKE_SOURCE_DIR}/../source/header/)
include_directories(testWebService $ENV{HIPE_EXTERNAL}/opencv/include)


target_link_libraries(testWebService ${CMAKE_SOURCE_DIR}/../build/target/lib/libWebRTCServerd.so)

target_link_libraries(testWebService ${OpenCV_LIBS})
target_link_libraries(testWebService -L${HIPE_EXTERNAL_DIR}/lib -lX11 -lXcomposite -lxcb -lXfixes -lXdamage ${CMAKE_SOURCE_DIR}/../../../install/lib/libssl.so ${CMAKE_SOURCE_DIR}/../../../install/lib/libcrypto.so /home/ddubuc/3rdParty/hipe_external/linux64/build/libwebrtc/build/webrtc/src/out/Release/obj/third_party/yasm/libyasm_utils.a -ldl libyasm.so -pthread)
