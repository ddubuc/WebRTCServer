#pragma once
#include <functional>
#include "server.h"


#include "internal/ConcurrentQueue.h"


std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnConnectHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseSenderHandler();
std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnCloseReceiverHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenSenderHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl)> OnOpenReceiverHandler(std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> stack);

std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageReceiverHandler();

std::function<void(WebsocketServer*, websocketpp::connection_hdl, message_ptr)> OnMessageSenderHandler(std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> i_stack);
