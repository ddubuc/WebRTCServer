#include "WebRTCCapturer.h"
#include <webrtc/api/peerconnectioninterface.h>
#include "internal/ConnectionData.h"
#include "internal/WebSocketHandler.h"
#include "internal/server.h"
#include "internal/ConcurrentQueue.h"



WebRTCCapturer::WebRTCCapturer(int i_port, const char *&workdir) : port(i_port)
{
	working_dir = strdup(workdir);
	stack = std::make_shared < core::queue::ConcurrentQueue<cv::Mat> >();
	ws = nullptr;
}

WebRTCCapturer::~WebRTCCapturer()
{
	stopWebRTCServer();
	free(working_dir);
	working_dir = nullptr;
}

int WebRTCCapturer::startWebRTCServer()
{
	webRTC_task = std::make_shared<std::thread>([this]
	{
		// mapping between socket connection and peer connection.
		std::unordered_map<void*, std::shared_ptr<ConnectionData>> connections;

		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory
			= webrtc::CreatePeerConnectionFactory();

		std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> l_stack;
		l_stack.reset(static_cast<core::queue::ConcurrentQueue<cv::Mat> *>(stack.get()), [](core::queue::ConcurrentQueue<cv::Mat> * ptr)
		{

		});
		WebsocketServer * _ws = static_cast<WebsocketServer *>(ws);
		{
			std::lock_guard<std::mutex> lock(safe_quard);
			
			std::stringstream base_cert;
			base_cert << this->working_dir;
			if (!base_cert.str().empty())
			{
#ifdef WIN32
				base_cert << "\\";
#else
				base_cert << "/";
#endif /* WIN32 */
			}
			base_cert << "mylaptop";

			_ws = ServerInit(
				// on http = on connection
				OnConnectHandler(),
				OnOpenReceiverHandler(connections, l_stack),
				OnCloseHandler(connections),
				// on message
				OnMessageSenderHandler(connections,
					std::bind([&]()
			{
			})),

			base_cert.str());
			ws = _ws;

			// Listen on port 9001
			_ws->listen(port);

			// Start the server accept loop
			_ws->start_accept();
		}
		LOG(INFO) << "run server" << port;
		_ws->run();
		{
			std::lock_guard<std::mutex> lock(safe_quard);
			std::error_code er;
			_ws->stop_listening(er);
		
			connections.clear();

			delete _ws;
			_ws = nullptr;
			ws = nullptr;
		}
	});

	int retry = 100000;

	while (--retry > 0 && ws == nullptr)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return (retry <= 0 && ws == nullptr) ? -1 : 0;
}

int WebRTCCapturer::stopWebRTCServer()
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
		webRTC_task->join();


	return 0;
}

cv::Mat WebRTCCapturer::Capture()
{
	cv::Mat ret;
	cv::Mat result;
	int retry = 3;

	{
		std::lock_guard<std::mutex> lock(safe_quard);
		std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> l_stack;
		l_stack.reset(static_cast<core::queue::ConcurrentQueue<cv::Mat> *>(stack.get()), [](core::queue::ConcurrentQueue<cv::Mat> *ptr)
		{
			
		});
		while (!static_cast<WebsocketServer *>(ws)->stopped())
		{
			if (!l_stack->trypop_until(ret, 300))
			{
				retry--;
				if (retry <= 0) //Timeout of 6 seconds ...the eternity so return en empty result;
					return result;

				continue;
			}
			
			if (!ret.empty() && (ret.size().height > 0 && ret.size().width > 0))
				break;
		}
		result = ret.clone();
	}
	return result;
}
