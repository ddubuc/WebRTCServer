#include <stdio.h>  /* defines FILENAME_MAX */
// #define WINDOWS  /* uncomment this line to use it for windows.*/ 
#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include<iostream>

#include "WebRTCStreamer.h"
#include <api/peerconnectioninterface.h>
#include "internal/ConnectionData.h"
#include "internal/WebSocketHandler.h"
#include "internal/CustomOpenCVCapturer.h"
#include "internal/server.h"
#include "internal/ConcurrentQueue.h"
#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <rtc_base/logging.h>

std::string GetCurrentWorkingDir(void) {
	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);
	return current_working_dir;
}

WebRTCStreamer::WebRTCStreamer(int i_port, const char * work_dir) : port(i_port)
{
	working_dir = strdup(work_dir);
	
	ws = nullptr;
	core::queue::ConcurrentQueue<cv::Mat> * l_stack = new core::queue::ConcurrentQueue<cv::Mat>();

	stack.reset(l_stack, [](void * ptr)
	{
		core::queue::ConcurrentQueue<cv::Mat> * l_stack = static_cast<core::queue::ConcurrentQueue<cv::Mat> *>(ptr);
		delete l_stack;
	});

	
}

int WebRTCStreamer::startWebRTCServer()
{
	std::string ser_dir = GetCurrentWorkingDir();

	webRTC_task = std::make_shared<std::thread>([this]
	{
		// mapping between socket connection and peer connection.
		std::unordered_map<void*, std::shared_ptr<ConnectionData>> connections;
		std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> l_stack;
		

		WebsocketServer* _ws = static_cast<WebsocketServer *>(ws);
		{
			std::lock_guard<std::mutex> lock(safe_quard);
			l_stack.reset(static_cast<core::queue::ConcurrentQueue<cv::Mat> *>(stack.get()), [] (core::queue::ConcurrentQueue<cv::Mat> *) {});

			std::stringstream base_cert;
			base_cert << this->working_dir;
			
			_ws = ServerInit(// on http = on connection
				OnConnectHandler(),
				OnOpenSenderHandler(),
				//on_close
				OnCloseSenderHandler(),
				// on message
				OnMessageSenderHandler(l_stack),
				base_cert.str());

		
			ws = _ws;


			// Listen on port 9001
			_ws->listen(port);

			// Start the server accept loop
			_ws->start_accept();
		} // safe lock guard

		RTC_LOG(INFO) << "run server " << port;
		_ws->run();
		{
			std::lock_guard<std::mutex> lock(safe_quard);
			std::error_code er;
			_ws->stop_listening(er);
			delete _ws;
			_ws = nullptr;
			ws = nullptr;
			
			connections.clear();

		
		}
	});

	int retry = 100;

	while (--retry > 0 && ws == nullptr)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	return (retry <= 0 && ws == nullptr) ? -1 : 0;
}

WebRTCStreamer::~WebRTCStreamer()
{
	free(working_dir);
	working_dir = nullptr;
	stopWebRTCServer();
}

int WebRTCStreamer::stopWebRTCServer()
{
	bool canStop = false;
	{
		std::lock_guard<std::mutex> lock(safe_quard);

		if (ws != nullptr && !static_cast<WebsocketServer *>(ws)->stopped())
			canStop = true;
	}
	if (canStop)
		static_cast<WebsocketServer *>(ws)->stop();

	if (webRTC_task && webRTC_task->joinable())
	{
		webRTC_task->join();
		webRTC_task.reset();
	}

	

	return 0;
}

void WebRTCStreamer::Send(const cv::Mat& mat)
{
	std::lock_guard<std::mutex> lock(safe_quard);
	core::queue::ConcurrentQueue<cv::Mat>* l_stack = static_cast<core::queue::ConcurrentQueue<cv::Mat> *>(stack.get());
	
	cv::Mat toSend;

	
	mat.copyTo(toSend);

	
	l_stack->clear();

	l_stack->push(toSend);
}

cWebStreamer newWebRTCStreamer(int port, const char * working_dir)
{
	return new WebRTCStreamer(port, working_dir);
}

void sendNewFrame(cWebStreamer ctx, uint8_t* data, int width, int height, int channel)
{
	cv::Mat mat;
	if (channel == 1)
		mat = cv::Mat(height, width, CV_8UC1, data);
	else if (channel == 3)
		mat = cv::Mat(height, width, CV_8UC3, data);
	else if (channel == 4)
		mat = cv::Mat(height, width, CV_8UC4, data);

	WebRTCStreamer*  This = static_cast<WebRTCStreamer*>(ctx);

	This->Send(mat);
}

void deleteWebRTCStreamer(cWebStreamer * ctx)
{
	WebRTCStreamer*  This = static_cast<WebRTCStreamer*>(*ctx);

	This->stopWebRTCServer();

	delete This;

	*ctx = nullptr;
}


void startStreamerServer(cWebStreamer ctx)
{
	WebRTCStreamer*  This = static_cast<WebRTCStreamer*>(ctx);

	This->startWebRTCServer();
}

void stopStreamerServer(cWebStreamer ctx)
{
	WebRTCStreamer*  This = static_cast<WebRTCStreamer*>(ctx);

	This->stopWebRTCServer();
}

