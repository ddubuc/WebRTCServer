#pragma once

#ifdef BOOST_PP_VARIADICS
#undef BOOST_PP_VARIADICS
#define BOOST_PP_VARIADICS 0
#endif

#include "websocketpp/config/asio.hpp"
#include "websocketpp/server.hpp"
#include <functional>
#include "PeerConnectionManager.h"

// pull out the type of messages sent by our config
typedef websocketpp::config::asio::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
//typedef websocketpp::server<websocketpp::config::asio> WebsocketServer;


class WebsocketServer : public websocketpp::server<websocketpp::config::asio_tls>
{
protected:
	std::function<void(WebsocketServer*, websocketpp::connection_hdl)> on_connection;
    std::function<void(WebsocketServer*, websocketpp::connection_hdl)> on_open;
    std::function<void(WebsocketServer*, websocketpp::connection_hdl)> on_close;
    std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> on_message;
	std::string basename;

public:
	WebsocketServer(std::string basename)
	{
		std::list<std::string> iceServerList;
		iceServerList.push_back(std::string("stun:stun.l.google.com:19302"));
		webrtc::AudioDeviceModule::AudioLayer audioLayer = webrtc::AudioDeviceModule::kDummyAudio;
		peerConnectionManager = std::make_shared<PeerConnectionManager>(iceServerList, audioLayer, ".*");
		init_asio();
		set_reuse_addr(true);
	}

	~WebsocketServer()
	{
		peerConnectionManager.reset();
	}

	std::shared_ptr<PeerConnectionManager> peer_connection_manager() const
	{
		return peerConnectionManager;
	};

	void onConnectionHandler(std::function<void(WebsocketServer*, websocketpp::connection_hdl)> handler)
	{
		on_connection = handler;
		this->set_http_handler([this]( websocketpp::connection_hdl hdl) { on_connection(this, hdl); });
	}


	void onOpenHandler(std::function<void(WebsocketServer*, websocketpp::connection_hdl)> openHandler)
	{
		on_open = openHandler;
		this->set_open_handler([this]( websocketpp::connection_hdl hdl) { on_open(this, hdl); });
	}

	void onCloseHandler(std::function<void(WebsocketServer*, websocketpp::connection_hdl)> handler)
	{
		on_close = handler;
		this->set_close_handler([this]( websocketpp::connection_hdl hdl) { on_close(this, hdl); });
	}

	
	void onMessageHandler(std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> i_on_message)
	{
		on_message = i_on_message;
		this->set_message_handler([this]( websocketpp::connection_hdl hdl, message_ptr message) { on_message(this, hdl, message); });
	}

protected:
	std::shared_ptr<PeerConnectionManager> peerConnectionManager;
};

WebsocketServer* ServerInit(std::function<void(WebsocketServer*, websocketpp::connection_hdl)> con_callback,
	std::function<void(WebsocketServer*, websocketpp::connection_hdl)> open_callback,
	std::function<void(WebsocketServer*, websocketpp::connection_hdl)> cls_callback,
	std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> msg_callback,
	std::string basename 
);


