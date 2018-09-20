#pragma once
#include <memory>
#include <thread>
#include <mutex>
#include <opencv2/core/mat.hpp>
#include "WebRTCServer_export.h"



class WEBRTCSERVER_EXPORT WebRTCCapturer
{
protected:
	int port;
	std::shared_ptr<std::thread> webRTC_task;
	std::mutex safe_quard;
	std::shared_ptr<void> stack;
	void* ws;
	char * working_dir;

public:
	WebRTCCapturer(int i_port, const char * work_dir);
	~WebRTCCapturer();

	int startWebRTCServer();

	int stopWebRTCServer();

	cv::Mat Capture();
};
