#include "internal/WebSocketHandler.h"
#include "internal/videorenderer.h"

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

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseReceiverHandler()
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl)
	{
		RTC_LOG(INFO) << "on_close ";
		std::string peerId = "VideoReceiver";
		
		s->peer_connection_manager()->hangUp(peerId);
	};

	return func;
}

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseSenderHandler()
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl)
	{
		RTC_LOG(INFO) << "on_close ";
		std::string peerId = "VideoSender";
		
		s->peer_connection_manager()->hangUp(peerId);
		
	};

	return func;
}



std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenSenderHandler()
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl)
	{
		RTC_LOG(INFO) << "on_open ";


		// create and set peer connection.
		std::shared_ptr<void> shared_ptr = hdl.lock();
		std::string peerId = "VideoSender";
	

		PeerConnectionManager::PeerConnectionObserver* peer_connection_observer = s->peer_connection_manager()->createClientOffer(peerId);

		peer_connection_observer->SetOnIceCandidate([&, shared_ptr](const webrtc::IceCandidateInterface* candidate)
		{
			std::string sdp;
			candidate->ToString(&sdp);
			RTC_LOG(INFO) << "send ice candidate... " << sdp;

			Json::Value jmessage;
			jmessage["candidate"]["sdpMid"] = candidate->sdp_mid();
			jmessage["candidate"]["sdpMLineIndex"] = candidate->sdp_mline_index();
			jmessage["candidate"]["candidate"] = sdp;

			std::string writeJson = Json::StyledWriter().write(jmessage);

			try
			{
				s->send(shared_ptr, writeJson, websocketpp::frame::opcode::text);
			}
			catch (const websocketpp::lib::error_code& e)
			{
				RTC_LOG(LS_ERROR) << "Echo failed because: " << e
					<< "(" << e.message() << ")";
			}

			RTC_LOG(INFO) << "send ice candidate end. ";
		});

		
	};


	return func;
}


inline void ProcessMessage()
{
	auto thread = rtc::Thread::Current();
	auto msg_cnt = thread->size();
	RTC_LOG(INFO) << "process message. : last " << msg_cnt;

	while (msg_cnt > 0)
	{
		rtc::Message msg;
		if (!thread->Get(&msg, 0))
			return;
		thread->Dispatch(&msg);
	}
}

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenReceiverHandler(std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> stack)
{
	auto func = [&, stack](WebsocketServer* s, websocketpp::connection_hdl hdl)
	{
		RTC_LOG(INFO) << "on_open ";
		std::string peerId = "VideoReceiver";
		PeerConnectionManager::PeerConnectionObserver* peer_connection_observer = s
		                                                                          ->peer_connection_manager()->
		                                                                          createClientOffer(peerId);

		std::shared_ptr<void> shared_ptr = hdl.lock();
		peer_connection_observer->setFuncOnAddStream(
			[&, stack](PeerConnectionManager::PeerConnectionObserver* peerConnectionObserver,
			    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
			{
				webrtc::VideoTrackVector tracks = stream->GetVideoTracks();
				if (tracks.size() > 0)
				{
					peerConnectionObserver->setVideosink(new VideoRenderer(1, 1, tracks[0], stack));
				}
			});


		peer_connection_observer->SetOnIceCandidate([&, shared_ptr](const webrtc::IceCandidateInterface* candidate)
		{
			std::string sdp;
			candidate->ToString(&sdp);
			RTC_LOG(INFO) << "send ice candidate... " << sdp;

			Json::Value jmessage;
			jmessage["candidate"]["sdpMid"] = candidate->sdp_mid();
			jmessage["candidate"]["sdpMLineIndex"] = candidate->sdp_mline_index();
			jmessage["candidate"]["candidate"] = sdp;
			std::string writeJson = Json::StyledWriter().write(jmessage);
			try
			{
				s->send(shared_ptr, writeJson, websocketpp::frame::opcode::text);
			}
			catch (const websocketpp::lib::error_code& e)
			{
				RTC_LOG(LS_ERROR) << "Echo failed because: " << e
					<< "(" << e.message() << ")";
			}

			RTC_LOG(INFO) << "send ice candidate end. ";
		});
	};

	return func;
}


std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageReceiverHandler()
{
	auto func = [&](WebsocketServer* s, websocketpp::connection_hdl hdl, message_ptr msg)
	{
		std::shared_ptr<void> shared_ptr = hdl.lock();
		RTC_LOG(INFO) << "on_message called with hdl: " << shared_ptr.get()
			<< " and message: " << msg->get_payload();
		std::string peerId = "VideoReceiver";
		PeerConnectionManager::PeerConnectionObserver* peer_connection_observer = s
		                                                                          ->peer_connection_manager()->
		                                                                          getPeerConnectionObserver(peerId);

		Json::Value request;
		std::string errors;
		std::string payload = msg->get_payload();
		const char* str = payload.c_str();
		Json::Reader().parse(str, request);

		// receive offer
		if (request["type"] == "offer")
		{
			RTC_LOG(INFO) << "1 -------------------- receive client offer ---------------------";

			std::string sdp = request["sdp"].asString();

			s->peer_connection_manager()->createAnswerToClientOffer(peerId, request,
			                                                        [&, shared_ptr](
			                                                        webrtc::SessionDescriptionInterface* desc)
			                                                        {
				                                                        std::string sdp;
				                                                        desc->ToString(&sdp);
				                                                        Json::Value answer;
				                                                        const char kSessionDescriptionTypeName[] =
					                                                        "type";
				                                                        const char kSessionDescriptionSdpName[] = "sdp";

				                                                        answer[kSessionDescriptionTypeName] = desc->
					                                                        type();
				                                                        answer[kSessionDescriptionSdpName] = sdp;
				                                                        RTC_LOG(INFO) <<
					                                                        "2 -------------------- send server answer ---------------------";
				                                                        try
				                                                        {
					                                                        s->send(
						                                                        shared_ptr,
						                                                        Json::StyledWriter().write(answer),
						                                                        msg->get_opcode());
				                                                        }
				                                                        catch (const websocketpp::lib::error_code& e)
				                                                        {
					                                                        RTC_LOG(LS_ERROR) << "Echo failed because: "
						                                                        << e
						                                                        << "(" << e.message() << ")";
				                                                        }
				                                                        RTC_LOG(INFO) << " create answer callback end.";


				                                                        // --- type == "offer" ---
			                                                        });;
		}
		else if (request["type"] == "join")
		{
			// create offer
			RTC_LOG(INFO) << " create offer start.";
			//s->peer_connection_manager()->joinClientOffer(peerId, request,
			//                                                        [&, shared_ptr](webrtc::SessionDescriptionInterface* desc)
			//                                                        {
			//	                                                        std::string sdp;
			//	                                                        desc->ToString(&sdp);
			//	                                                        Json::Value answer;
			//	                                                        const char kSessionDescriptionTypeName[] = "type";
			//	                                                        const char kSessionDescriptionSdpName[] = "sdp";

			//	                                                        answer[kSessionDescriptionTypeName] = desc->type();
			//	                                                        answer[kSessionDescriptionSdpName] = sdp;
			//	                                                        RTC_LOG(INFO) <<
			//		                                                        "2 -------------------- send server answer ---------------------";
			//	                                                        try
			//	                                                        {
			//		                                                        s->send(
			//			                                                        shared_ptr, Json::StyledWriter().write(answer),
			//			                                                        msg->get_opcode());
			//	                                                        }
			//	                                                        catch (const websocketpp::lib::error_code& e)
			//	                                                        {
			//		                                                        RTC_LOG(LS_ERROR) << "Echo failed because: "
			//			                                                        << e
			//			                                                        << "(" << e.message() << ")";
			//	                                                        }
			//	                                                        RTC_LOG(INFO) << " create answer callback end.";


			//	                                                        // --- type == "offer" ---
			//                                                        });
		}
		else if (request["type"] == "answer")
		{
			// receive answer
			RTC_LOG(LS_ERROR) << "4 -------------------- receive client answer ---------------------";
			throw std::runtime_error("Cannot receive Answer resquest from Client when server ask for video");
		}
		else if (request.isMember("candidate"))
		{
			RTC_LOG(INFO) << "on ice candidate.";

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
				RTC_LOG(LS_WARNING) << "Can't parse received candidate message. "
					<< "SdpParseError was: " << error.description;
				return;
			}
			if (!peer_connection_observer->getPeerConnection()->AddIceCandidate(candidate.get()))
			{
				RTC_LOG(LS_WARNING) << "Failed to apply the received candidate";
				return;
			}
			RTC_LOG(INFO) << "set ice  candidate end.";
		}
		else if (request.isMember("mode"))
		{
		}
		ProcessMessage();
		RTC_LOG(INFO) << "end of request.";
	}; // on_message

	return func;
}

std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageSenderHandler(std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> i_stack)
{
	auto func = [&, i_stack](WebsocketServer* s, websocketpp::connection_hdl hdl, message_ptr msg)
	{
		std::shared_ptr<void> shared_ptr = hdl.lock();
		RTC_LOG(INFO) << "on_message called with hdl: " << shared_ptr.get()
			<< " and message: " << msg->get_payload();
		std::string peerId = "VideoSender";
		PeerConnectionManager::PeerConnectionObserver* peer_connection_observer = s
		                                                                          ->peer_connection_manager()->
		                                                                          getPeerConnectionObserver(peerId);


		// receive as JSON.

		Json::Value request;
		std::string errors;
		std::string payload = msg->get_payload();
		const char* str = payload.c_str();
		Json::Reader().parse(str, request);

		//// receive offer
		if (request["type"] == "offer")
		{
			RTC_LOG(INFO) << "1 -------------------- receive client offer ---------------------";
			throw std::runtime_error("shouldn't receive an offer when server ask to stream video");

			std::string sdp = request["sdp"].asString();
			webrtc::SdpParseError error;
			webrtc::SessionDescriptionInterface* session_description(
				webrtc::CreateSessionDescription("offer", sdp, &error));
			if (!session_description)
			{
				RTC_LOG(LS_WARNING) << "Can't parse received session description message. "
					<< "SdpParseError was: " << error.description;
				return;
			}


			RTC_LOG(INFO) << " set remote offer description start.";
			/*peer_connection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);*/
			RTC_LOG(INFO) << " set remote offer description end.";

			// send answer
			RTC_LOG(INFO) << " create answer start.";
			

			// --- type == "offer" ---
		}
		else if (request["type"] == "join")
		{
			// create offer
			RTC_LOG(LS_WARNING) << "Deprecated function please remove it from your request call offer and answer instead";
		}
		else if (request["type"] == "answer")
		{
			// receive answer
			RTC_LOG(INFO) << "4 -------------------- receive client answer ---------------------";
			s->peer_connection_manager()->setAnswer(peerId, request);
		
		}
		else if (request.isMember("candidate"))
		{
			RTC_LOG(INFO) << "on ice candidate.";

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
				RTC_LOG(LS_WARNING) << "Can't parse received candidate message. "
					<< "SdpParseError was: " << error.description;
				return;
			}
			if (!peer_connection_observer->getPeerConnection()->AddIceCandidate(candidate.get()))
			{
				RTC_LOG(LS_WARNING) << "Failed to apply the received candidate";
				return;
			}
			RTC_LOG(INFO) << "set ice  candidate end.";
		}
		else if (request.isMember("mode"))
		{
			if (request["mode"]["onAir"].asBool())
			{
				std::string options;
				s->peer_connection_manager()->createOffer(peerId, options,
				                                          i_stack, [&, shared_ptr](webrtc::SessionDescriptionInterface* desc)
				                                          {
					                                          Json::Value offer;
					                                          // Names used for a SessionDescription JSON object.
					                                          const char kSessionDescriptionTypeName[] = "type";
					                                          const char kSessionDescriptionSdpName[] = "sdp";
					                                          if (desc)
					                                          {
						                                          std::string sdp;
						                                          desc->ToString(&sdp);

						                                          offer[kSessionDescriptionTypeName] = desc->type();
						                                          offer[kSessionDescriptionSdpName] = sdp;
					                                          }
					                                          else
					                                          {
						                                          RTC_LOG(LS_ERROR) << "Failed to create offer";
					                                          }
					                                          RTC_LOG(INFO) <<
						                                          "3 -------------------- send server offer ---------------------";
					                                          try
					                                          {
						                                          std::string offer_str = Json::StyledWriter().write(offer);
						                                          RTC_LOG(LS_VERBOSE) << " sending offer..." << offer_str;
						                                          s->send(shared_ptr, offer_str,
						                                                  websocketpp::frame::opcode::value::TEXT);
					                                          }
					                                          catch (const websocketpp::lib::error_code& e)
					                                          {
						                                          RTC_LOG(LS_ERROR) << "Echo failed because: " << e << "(" << e.
							                                          message() << ")";
					                                          }
				                                          });	
				
			}
			else
			{
				s->peer_connection_manager()->stopOpenCVStreaming(peerId);
			}
		}
		ProcessMessage();
		RTC_LOG(INFO) << "end of request.";
	}; // on_message

		return func;
		}
