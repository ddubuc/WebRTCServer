#pragma once
#include <memory>
#include <websocketpp/transport/base/connection.hpp>
#include "videorenderer.h"
#include "webrtc.h"



struct ConnectionData
{
	websocketpp::connection_hdl hdl;

	ConnectionData(
	
		websocketpp::connection_hdl h
	) : hdl(h)
	{
	}

	~ConnectionData()
	{

		RTC_LOG(INFO) << "~ConnectionData()";
	}
};
