#pragma once
#include <memory>
#include <thread>
#include <mutex>
#include <opencv2/core/mat.hpp>
#include "WebRTCServer_export.h"

class WEBRTCSERVER_EXPORT WebRTCStreamer
{
protected:
	int port;
	std::shared_ptr<std::thread> webRTC_task;
	std::mutex safe_quard;
	std::shared_ptr<void> stack;
	void* ws;
	char * working_dir;
	char * base_cert;
	void *_contextWebRTC;

public:
	WebRTCStreamer(int i_port, const char * work_dir, const char * i_base_cert);

	~WebRTCStreamer();

	int startWebRTCServer();

	int stopWebRTCServer();

	void Send(const cv::Mat& mat);

};

typedef void * cWebStreamer;

WEBRTCSERVER_EXPORT cWebStreamer newWebRTCStreamer(int port, const char * workdir, const char *base_cert);

WEBRTCSERVER_EXPORT void sendNewFrame(cWebStreamer ctx, uint8_t* data, int width, int height, int channel);

WEBRTCSERVER_EXPORT void deleteWebRTCStreamer(cWebStreamer * ctx);

WEBRTCSERVER_EXPORT void startStreamerServer(cWebStreamer ctx);

WEBRTCSERVER_EXPORT void stopStreamerServer(cWebStreamer ctx);

