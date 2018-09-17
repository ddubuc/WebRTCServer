#include "internal/WebSocketHandler.h"

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnConnectHandler()
{
	auto func = [](WebsocketServer* s, websocketpp::connection_hdl hdl)
	{
		WebsocketServer::connection_ptr con = s->get_con_from_hdl(hdl);

		con->set_body("Hello World!");
		con->set_status(websocketpp::http::status_code::ok);
	};

	return func;
}

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections)
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl)
	{
		LOG(INFO) << "on_close ";
		auto& connection = connections[hdl.lock().get()];
		connection->pc->Close();
		
		connections.erase(hdl.lock().get());
	};

	return func;
}

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenSenderHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections)
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl)
	{
		LOG(INFO) << "on_open ";

		// create and set peer connection.
		auto callback = new PeerConnectionCallback();
		connections[hdl.lock().get()] = ::make_unique<ConnectionData>(
			CreatePeerConnection(callback),
			callback,
			hdl
		);
		auto& connection = connections[hdl.lock().get()];
		auto& peer_connection = connection->pc;
		auto& peer_connection_callback = connection->callback;

		peer_connection_callback->SetOnAddStream(
			[&](rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
		{
			//webrtc::VideoTrackVector tracks = stream->GetVideoTracks();
			LOG(INFO) << "No input source needed";
		});
		//send ice candidate...
		peer_connection_callback->SetOnIceCandidate([&](const webrtc::IceCandidateInterface* candidate)
		{
			std::string sdp;
			candidate->ToString(&sdp);
			LOG(INFO) << "send ice candidate... " << sdp;

			Json::StreamWriterBuilder writer;
			Json::Value jmessage;

			jmessage["candidate"]["sdpMid"] = candidate->sdp_mid();
			jmessage["candidate"]["sdpMLineIndex"] = candidate->sdp_mline_index();
			jmessage["candidate"]["candidate"] = sdp;

			try
			{
				s->send(connection->hdl, writeString(writer, jmessage), websocketpp::frame::opcode::text);
			}
			catch (const websocketpp::lib::error_code& e)
			{
				LOG(LERROR) << "Echo failed because: " << e
					<< "(" << e.message() << ")";
			}

			LOG(INFO) << "send ice candidate end. ";
		});

		peer_connection_callback->SetOnNegotiation([&]()
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
					                             s->send(connection->hdl, writeString(writer, jmessage), websocketpp::frame::opcode::value::TEXT);
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
		});

		peer_connection_callback->SetOnSignalingChange([&](webrtc::PeerConnectionInterface::SignalingState new_state)
		{
			if (new_state == webrtc::PeerConnectionInterface::SignalingState::kClosed)
			{
				peer_connection->Close();
			}
		});
	};
	/*std::function<void(WebsocketServer*, websocketpp::connection_hdl)> f = func;*/

	return func;
}

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenReceiverHandler(std::unordered_map<void*, std::shared_ptr<ConnectionData>>& connections, std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> & stack)
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl)
	{
		LOG(INFO) << "on_open ";

		// create and set peer connection.
		auto callback = new PeerConnectionCallback();
		connections[hdl.lock().get()] = ::make_unique<ConnectionData>(
			CreatePeerConnection(callback),
			callback,
			hdl
		);
		auto& connection = connections[hdl.lock().get()];

		auto& peer_connection = connection->pc;
		auto& peer_connection_callback = connection->callback;

		peer_connection_callback->SetOnAddStream(
			[&](rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
		{
			webrtc::VideoTrackVector tracks = stream->GetVideoTracks();

			connection->renderer.reset(new VideoRenderer(1, 1, tracks[0], stack));
		
			std::cout << "Video track is created" << std::endl;
	
		});


		peer_connection_callback->SetOnIceCandidate([&](const webrtc::IceCandidateInterface* candidate)
		{
			std::string sdp;
			candidate->ToString(&sdp);
			LOG(INFO) << "send ice candidate... " << sdp;

			Json::StreamWriterBuilder writer;
			Json::Value jmessage;

			jmessage["candidate"]["sdpMid"] = candidate->sdp_mid();
			jmessage["candidate"]["sdpMLineIndex"] = candidate->sdp_mline_index();
			jmessage["candidate"]["candidate"] = sdp;

			try
			{
				s->send(connection->hdl, writeString(writer, jmessage), websocketpp::frame::opcode::text);
			}
			catch (const websocketpp::lib::error_code& e)
			{
				LOG(LERROR) << "Echo failed because: " << e
					<< "(" << e.message() << ")";
			}

			LOG(INFO) << "send ice candidate end. ";
		});

		peer_connection_callback->SetOnSignalingChange([&](webrtc::PeerConnectionInterface::SignalingState new_state)
		{
			if (new_state == webrtc::PeerConnectionInterface::SignalingState::kClosed)
			{
				peer_connection->Close();
			}
		});
	};

	return func;
}
