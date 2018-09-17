#pragma once
#include <functional>
#include "server.h"
#include "ConnectionData.h"

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnConnectHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections);

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenSenderHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections);

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenReceiverHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections,
                                                                                                std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> & stack);
#ifdef WIN32																											  
template<class _Fx,
	class... _Types>
inline std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageSenderHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>> & connections,
	std::_Binder<std::_Unforced, _Fx, _Types...> bindOnAir)
#else
template<typename _Fx,
	typename... _Types>
inline std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageSenderHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>> & connections,
    std::_Bind<_Fx(_Types...)> bindOnAir)
#endif	
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
		Json::Reader reader;
		Json::Value request;
		if (!reader.parse(msg->get_payload(), request))
		{
			LOG(WARNING) << "Received unknown message. " << msg->get_payload();
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
				//SendVideo(connection, peer_connection_factory, stack);
				std::function<void(std::shared_ptr<ConnectionData>)> funcOnAir = bindOnAir;
				funcOnAir(connection);
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
