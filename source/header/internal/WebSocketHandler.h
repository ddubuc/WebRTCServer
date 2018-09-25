#pragma once
#include <functional>
#include "server.h"
#include "ConnectionData.h"
#include "internal/CustomOpenCVCapturer.h"
#include "internal/ConcurrentQueue.h"
#include "json/include/json/json.h"

inline void SendVideo(std::shared_ptr<ConnectionData> connection, std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat> > stack, rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory)
{
	auto& peer_connection = connection->pc;

	std::unique_ptr<CustomOpenCVCapturer> capturer = ::make_unique<CustomOpenCVCapturer>(stack);
	rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> videoSource = peer_connection_factory->CreateVideoSource(std::move(capturer), nullptr);
	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track = peer_connection_factory->CreateVideoTrack("videoSEND_label", videoSource);
	rtc::scoped_refptr<webrtc::MediaStreamInterface> new_stream = peer_connection_factory->CreateLocalMediaStream("streamSEND_label");

	//for (auto &track : stream->GetAudioTracks()) { new_stream->AddTrack(track); }
	
	new_stream->AddTrack(video_track);
	if (!peer_connection->AddStream(new_stream))
	{
		LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
	}
	// connection->new_stream = new_stream;
	// connection->video_track = video_track;
	//connection->videoSource = videoSource;
}


std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnConnectHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections);

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenSenderHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections, rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory);

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenReceiverHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections,
                                                                                                std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> & stack);

inline std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageReceiverHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>> & connections)
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl, message_ptr msg)
	{
		LOG(INFO) << "on_message called with hdl: " << hdl.lock().get()
			<< " and message: " << msg->get_payload()
			<< std::endl;

		auto& connection = connections[hdl.lock().get()];
		connection->hdl = hdl;
		auto& peer_connection = connection->pc;

		// receive as JSON.
		Json::CharReaderBuilder builder;
		Json::CharReader * reader = builder.newCharReader();
		Json::Value request;
		std::string errors;
		std::string payload = msg->get_payload();
		const char * str = payload.c_str();
		if (!reader->parse(str, str + payload.size(), &request, nullptr))
		{
			LOG(WARNING) << "Received unknown message. " << payload;
			LOG(WARNING) << "Json parsing error message. " << errors;
			return;
		}

		// receive offer
		if (request["type"] == "offer")
		{
			LOG(INFO) << "1 -------------------- receive client offer ---------------------";

			std::string sdp = request["sdp"].asString();
			webrtc::SdpParseError error;
			webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("offer", sdp, &error));
			if (!session_description)
			{
				LOG(WARNING) << "Can't parse received session description message. "
					<< "SdpParseError was: " << error.description;
				return;
			}


			LOG(INFO) << " set remote offer description start.";
			peer_connection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
			LOG(INFO) << " set remote offer description end.";

			// send answer
			LOG(INFO) << " create answer start.";
			peer_connection->CreateAnswer(new rtc::RefCountedObject<CreateSDPCallback>(
				[&](webrtc::SessionDescriptionInterface* desc)
			{
				LOG(INFO) << " create answer callback start.";
				peer_connection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

				std::string sdp;
				desc->ToString(&sdp);

				Json::StreamWriterBuilder writer;
				Json::Value jmessage;
				jmessage["type"] = desc->type();
				jmessage["sdp"] = sdp;

				LOG(INFO) << "2 -------------------- send server answer ---------------------";
				try
				{
					LOG(INFO) << " sending answer..." << writeString(writer, jmessage);
					s->send(hdl, writeString(writer, jmessage), msg->get_opcode());
				}
				catch (const websocketpp::lib::error_code& e)
				{
					LOG(LERROR) << "Echo failed because: " << e
						<< "(" << e.message() << ")";
				}
				LOG(INFO) << " create answer callback end.";
			}, nullptr)
				, nullptr);

			// --- type == "offer" ---
		}
		else if (request["type"] == "join")
		{
			// create offer
			LOG(INFO) << " create offer start.";

			peer_connection->CreateOffer(new rtc::RefCountedObject<CreateSDPCallback>(
				[&](webrtc::SessionDescriptionInterface* desc)
			{
				LOG(INFO) << " create offer callback start.";
				peer_connection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

				// send SDP. trickle ICE.
				std::string sdp;
				desc->ToString(&sdp);

				Json::StreamWriterBuilder writer;
				Json::Value jmessage;
				jmessage["type"] = desc->type();
				jmessage["sdp"] = sdp;

				LOG(INFO) << "3 -------------------- send server offer ---------------------";
				try
				{
					LOG(INFO) << " sending offer..." << writeString(writer, jmessage);
					s->send(hdl, writeString(writer, jmessage), msg->get_opcode());
				}
				catch (const websocketpp::lib::error_code& e)
				{
					LOG(LERROR) << "Echo failed because: " << e
						<< "(" << e.message() << ")";
				}

				LOG(INFO) << " create offer callback start.";
			},
				nullptr
				), nullptr);
		}
		else if (request["type"] == "answer")
		{
			// receive answer
			LOG(INFO) << "4 -------------------- receive client answer ---------------------";
			std::string sdp = request["sdp"].asString();

			webrtc::SdpParseError error;
			webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("answer", sdp, &error));
			if (!session_description)
			{
				LOG(WARNING) << "Can't parse received session description message. "
					<< "SdpParseError was: " << error.description;
				return;
			}
			LOG(INFO) << " set remote answer description start.";
			peer_connection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
			LOG(INFO) << " set remote answer description end.";
		}
		else if (request.isMember("candidate"))
		{
			LOG(INFO) << "on ice candidate.";

			webrtc::SdpParseError error;
			std::unique_ptr<webrtc::IceCandidateInterface> candidate(
				webrtc::CreateIceCandidate(
					request["candidate"]["sdpMid"].asString(),
					request["candidate"]["sdpMLineIndex"].asInt(),
					request["candidate"]["candidate"].asString(),
					&error)
			);
			if (!candidate.get())
			{
				LOG(WARNING) << "Can't parse received candidate message. "
					<< "SdpParseError was: " << error.description;
				return;
			}
			if (!peer_connection->AddIceCandidate(candidate.get()))
			{
				LOG(WARNING) << "Failed to apply the received candidate";
				return;
			}
			LOG(INFO) << "set ice  candidate end.";
		}
		else if (request.isMember("mode"))
		{
			connection->session.mode.onAir = request["mode"]["onAir"].asBool();
			connection->session.mode.isKaicho = request["mode"]["isKaicho"].asBool();
			connection->session.mode.useEye = request["mode"]["useEye"].asBool();
			connection->session.mode.useMustache = request["mode"]["useMustache"].asBool();
		}

		ProcessMessage();
		LOG(INFO) << "end of request.";
	}; // on_message

	return func;
}

inline std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageSenderHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>> & connections, std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> i_stack, 				rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory)
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl, message_ptr msg)
	{
		LOG(INFO) << "on_message called with hdl: " << hdl.lock().get()
			<< " and message: " << msg->get_payload()
			<< std::endl;

		auto& connection = connections[hdl.lock().get()];
		connection->hdl = hdl;
		auto& peer_connection = connection->pc;

		// receive as JSON.
		Json::CharReaderBuilder builder;
		Json::CharReader * reader = builder.newCharReader();
		Json::Value request;
		std::string errors;
		std::string payload = msg->get_payload();
		const char * str = payload.c_str();
		if (!reader->parse(str, str + payload.size(), &request, nullptr))
		{
			LOG(WARNING) << "Received unknown message. " << payload;
			LOG(WARNING) << "Json parsing error message. " << errors;
			return;
		}

		// receive offer
		if (request["type"] == "offer")
		{
			LOG(INFO) << "1 -------------------- receive client offer ---------------------";

			std::string sdp = request["sdp"].asString();
			webrtc::SdpParseError error;
			webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("offer", sdp, &error));
			if (!session_description)
			{
				LOG(WARNING) << "Can't parse received session description message. "
					<< "SdpParseError was: " << error.description;
				return;
			}


			LOG(INFO) << " set remote offer description start.";
			peer_connection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
			LOG(INFO) << " set remote offer description end.";

			// send answer
			LOG(INFO) << " create answer start.";
			peer_connection->CreateAnswer(new rtc::RefCountedObject<CreateSDPCallback>(
				[&](webrtc::SessionDescriptionInterface* desc)
			{
				LOG(INFO) << " create answer callback start.";
				peer_connection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

				//std::string sdp;
				desc->ToString(&sdp);

				Json::StreamWriterBuilder writer;
				Json::Value jmessage;
				jmessage["type"] = desc->type();
				jmessage["sdp"] = sdp;

				LOG(INFO) << "2 -------------------- send server answer ---------------------";
				try
				{
					LOG(INFO) << " sending answer..." << writeString(writer, jmessage);
					s->send(hdl, writeString(writer, jmessage), msg->get_opcode());
				}
				catch (const websocketpp::lib::error_code& e)
				{
					LOG(LERROR) << "Echo failed because: " << e
						<< "(" << e.message() << ")";
				}
				LOG(INFO) << " create answer callback end.";
			}, nullptr)
				, nullptr);

			// --- type == "offer" ---
		}
		else if (request["type"] == "join")
		{
			// create offer
			LOG(INFO) << " create offer start.";

			peer_connection->CreateOffer(new rtc::RefCountedObject<CreateSDPCallback>(
				[&](webrtc::SessionDescriptionInterface* desc)
			{
				LOG(INFO) << " create offer callback start.";
				peer_connection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

				// send SDP. trickle ICE.
				std::string sdp;
				desc->ToString(&sdp);

				Json::StreamWriterBuilder writer;
				Json::Value jmessage;
				jmessage["type"] = desc->type();
				jmessage["sdp"] = sdp;

				LOG(INFO) << "3 -------------------- send server offer ---------------------";
				try
				{
					LOG(INFO) << " sending offer..." << writeString(writer, jmessage);
					s->send(hdl, writeString(writer, jmessage), msg->get_opcode());
				}
				catch (const websocketpp::lib::error_code& e)
				{
					LOG(LERROR) << "Echo failed because: " << e
						<< "(" << e.message() << ")";
				}

				LOG(INFO) << " create offer callback start.";
			},
				nullptr
				), nullptr);
		}
		else if (request["type"] == "answer")
		{
			// receive answer
			LOG(INFO) << "4 -------------------- receive client answer ---------------------";
			std::string sdp = request["sdp"].asString();

			webrtc::SdpParseError error;
			webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("answer", sdp, &error));
			if (!session_description)
			{
				LOG(WARNING) << "Can't parse received session description message. "
					<< "SdpParseError was: " << error.description;
				return;
			}
			LOG(INFO) << " set remote answer description start.";
			peer_connection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
			LOG(INFO) << " set remote answer description end.";
		}
		else if (request.isMember("candidate"))
		{
			LOG(INFO) << "on ice candidate.";

			webrtc::SdpParseError error;
			std::unique_ptr<webrtc::IceCandidateInterface> candidate(
				webrtc::CreateIceCandidate(
					request["candidate"]["sdpMid"].asString(),
					request["candidate"]["sdpMLineIndex"].asInt(),
					request["candidate"]["candidate"].asString(),
					&error)
			);
			if (!candidate.get())
			{
				LOG(WARNING) << "Can't parse received candidate message. "
					<< "SdpParseError was: " << error.description;
				return;
			}
			if (!peer_connection->AddIceCandidate(candidate.get()))
			{
				LOG(WARNING) << "Failed to apply the received candidate";
				return;
			}
			LOG(INFO) << "set ice  candidate end.";
		}
		else if (request.isMember("mode"))
		{
			connection->session.mode.onAir = request["mode"]["onAir"].asBool();
			if (connection->session.mode.onAir)
			{
				SendVideo(connection, i_stack, peer_connection_factory);
			}
			connection->session.mode.isKaicho = request["mode"]["isKaicho"].asBool();
			connection->session.mode.useEye = request["mode"]["useEye"].asBool();
			connection->session.mode.useMustache = request["mode"]["useMustache"].asBool();
		}

		ProcessMessage();
		LOG(INFO) << "end of request.";
	}; // on_message

	return func;
}
