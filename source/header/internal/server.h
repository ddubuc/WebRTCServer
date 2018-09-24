#pragma once

#ifdef BOOST_PP_VARIADICS
#undef BOOST_PP_VARIADICS
#define BOOST_PP_VARIADICS 0
#endif

#include "websocketpp/config/asio.hpp"
#include "websocketpp/server.hpp"

// pull out the type of messages sent by our config
typedef websocketpp::config::asio::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
typedef websocketpp::server<websocketpp::config::asio> WebsocketServer;

WebsocketServer* ServerInit(
    std::function<void(WebsocketServer*, websocketpp::connection_hdl)> on_connection,
    std::function<void(WebsocketServer*, websocketpp::connection_hdl)> on_open,
    std::function<void(WebsocketServer*, websocketpp::connection_hdl)> on_close,
    std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> on_message,
	std::string basename
);


