#include "internal/webrtc.h"
		 
#include "internal/server.h"
#include "internal/entrypoint.h"
#include "internal/customvideocapturer.h"
#include "internal/CustomOpenCVCapturer.h"

//
//void SendVideo(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory,
//               std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> stack,
//               std::shared_ptr<ConnectionData> connection)
//{
//	auto& peer_connection = connection->pc;
//	auto& peer_connection_callback = connection->callback;
//	//std::unique_ptr<CustomOpenCVCapturer> capturer());
//	rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> videoSource = peer_connection_factory->CreateVideoSource(new CustomOpenCVCapturer(connection->session, stack));
//	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track = peer_connection_factory->CreateVideoTrack("videoSEND_label", videoSource);
//	rtc::scoped_refptr<webrtc::MediaStreamInterface> new_stream = peer_connection_factory->CreateLocalMediaStream("streamSEND_label");
//
//	//for (auto &track : stream->GetAudioTracks()) { new_stream->AddTrack(track); }
//
//	new_stream->AddTrack(video_track);
//	if (!peer_connection->AddStream(new_stream))
//	{
//		LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
//	}
//}
//
//
//int entryPoint()
//{
//	// mapping between socket connection and peer connection.
//	std::unordered_map<void*, std::shared_ptr<ConnectionData>> connections;
//	std::unordered_map<void*, std::shared_ptr<ConnectionData>> connectionsRCV;
//
//
//	std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> stack = std::make_shared<core::queue::ConcurrentQueue<cv::Mat>>();
//
//
//	std::shared_ptr<std::thread> task1 = std::make_shared<std::thread>([&connections, stack]
//	{
//		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory
//			= webrtc::CreatePeerConnectionFactory();
//		int port = 9001;
//		WebsocketServer* ws = ServerInit(
//		                                 // on http = on connection
//		                                 OnConnectHandler(),
//		                                 OnOpenReceiverHandler(connections, stack),
//		                                 OnCloseHandler(connections),
//		                                 // on message
//		                                 OnMessageSenderHandler(connections,
//		                                                        std::bind([&]()
//		                                                        {
//		                                                        }))
//		);
//		// Listen on port 9001
//		ws->listen(port);
//
//		// Start the server accept loop
//		ws->start_accept();
//		LOG(INFO) << "run server 9001";
//		ws->run();
//	});
//
//	std::shared_ptr<std::thread> task2 = std::make_shared<std::thread>([&connectionsRCV, stack]
//	{
//		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory
//			= webrtc::CreatePeerConnectionFactory();
//		int port = 9002;
//		WebsocketServer* ws2 = ServerInit(
//					// on http = on connection
//		                                  OnConnectHandler(),
//		                                  OnOpenSenderHandler(connectionsRCV),
//		                                  //on_close
//		                                  OnCloseHandler(connectionsRCV),
//		                                  // on message
//		                                  OnMessageSenderHandler(connectionsRCV,
//		                                                         std::bind(&SendVideo, peer_connection_factory, stack, std::tr1::placeholders::_1))
//		);
//		// Listen on port 9002
//		ws2->listen(port);
//
//		// Start the server accept loop
//		ws2->start_accept();
//
//		LOG(INFO) << "run server 9002";
//		ws2->run();
//	});
//	task2->join();
//
//
//	return 0;
//};
//

extern "C"
{
#ifndef WIN32
#define ENTRYPOINT __attribute__((constructor))
#define DELETEPOINT __attribute__((destructor))
#else
#define ENTRYPOINT WEBRTCSERVER_EXPORT
#define DELETEPOINT WEBRTCSERVER_EXPORT
#endif


	ENTRYPOINT void load()
	{
		// rtc::LogMessage::LogToDebug(rtc::LS_ERROR);

		rtc::InitializeSSL();
	}

	DELETEPOINT void unload()
	{
		rtc::CleanupSSL();
	}

#ifdef WIN32
#include <windows.h>

	BOOL WINAPI DllMain(
		HINSTANCE hinstDLL, // handle to DLL module
		DWORD fdwReason, // reason for calling function
		LPVOID lpReserved) // reserved
	{
		// Perform actions based on the reason for calling.
		switch (fdwReason)
		{
		case DLL_PROCESS_ATTACH:
			// Initialize once for each new process.
			// Return FALSE to fail DLL load.
			load();
			break;

		case DLL_THREAD_ATTACH:
			// Do thread-specific initialization.
			break;

		case DLL_THREAD_DETACH:
			// Do thread-specific cleanup.
			break;

		case DLL_PROCESS_DETACH:
			// Perform any necessary cleanup.
			unload();
			break;
		}
		return TRUE; // Successful DLL_PROCESS_ATTACH.
	}
#endif
}
