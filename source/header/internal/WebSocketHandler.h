#pragma once
#include <functional>
#include "server.h"


#include "internal/ConcurrentQueue.h"


//
//inline void SendVideo(std::shared_ptr<ConnectionData> connection, std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat> > stack, rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory)
//{
//	auto& peer_connection = connection->pc;
//
//	std::unique_ptr<CustomOpenCVCapturer> capturer = ::make_unique<CustomOpenCVCapturer>(stack);
//	rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> videoSource = peer_connection_factory->CreateVideoSource(std::move(capturer), nullptr);
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
//	// connection->new_stream = new_stream;
//	// connection->video_track = video_track;
//	//connection->videoSource = videoSource;
//}


std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnConnectHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseSenderHandler();
std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseReceiverHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenSenderHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenReceiverHandler(std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> stack);

std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageReceiverHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageSenderHandler(std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> i_stack);
