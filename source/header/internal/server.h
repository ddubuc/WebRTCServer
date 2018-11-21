#pragma once

#ifdef BOOST_PP_VARIADICS
#undef BOOST_PP_VARIADICS
#define BOOST_PP_VARIADICS 0
#endif

#include "websocketpp/config/asio.hpp"
#include "websocketpp/server.hpp"
#include <functional>
#include "PeerConnectionManager.h"
#include <vector>
#include <string>

// pull out the type of messages sent by our config
typedef websocketpp::config::asio::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;


class RTCWebScoketServer : public websocketpp::server<websocketpp::config::asio_tls>
{
public:
	typedef websocketpp::server<websocketpp::config::asio_tls>::connection_ptr connection_ptr;

protected:
	std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> on_connection;
    std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> on_open;
    std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> on_close;
    std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl, message_ptr)> on_message;
	std::string basename;

public:
	RTCWebScoketServer(std::string basename)
	{
		std::list<std::string> iceServerList;
		iceServerList.push_back(std::string("stun:stun.l.google.com:19302"));
		webrtc::AudioDeviceModule::AudioLayer audioLayer = webrtc::AudioDeviceModule::kDummyAudio;
		peerConnectionManager = std::make_shared<PeerConnectionManager>(iceServerList, audioLayer, ".*");
		init_asio();
		set_reuse_addr(true);
	}

	~RTCWebScoketServer()
	{
		std::lock_guard<std::mutex> lock(locker);

		peerConnectionManager.reset();
	}

	std::vector<std::string> parseInputString(std::string &inp, std::string delimiter, bool isAllParts) {
        // Если isAllParts == true в вектор помещаются все части входящей строки
        // Иначе все кроме первой (для команд и аттрибутов первая часть содержит имя)
        // По умолчанию isAllParts == false
        // Из входящей строки inp вырезаются всё, что расположено после delimiter
		std::string s = inp;
        //std::string delimiter = ";";
        std::string token;
        std::string nameAttr;
        std::vector<std::string> parsed;

        size_t pos = 0;
        bool firstiter = true;

        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            if (firstiter) {
                firstiter = false;
                nameAttr = token;
            }
            else
                parsed.push_back(token);
            s.erase(0, pos + delimiter.length());
        }

        if (!firstiter)
            parsed.push_back(s);

        if (parsed.size() == 0) {
            if (isAllParts)
                parsed.insert(parsed.begin(), inp);
                //parsed.push_back(inp);
            inp = s;
            return parsed;
        }
        inp = nameAttr;
        if (isAllParts)
            parsed.insert(parsed.begin(), inp);
            //parsed.push_back(inp);

        return parsed;
    }

	std::string parseOfAddress(std::string addrFromConn) {
		std::string out = "";
		std::string tmpFnd;

        std::size_t found = addrFromConn.find("]");
        if (found == std::string::npos)
            return out;

        tmpFnd = addrFromConn.substr(0, found);

        found = tmpFnd.find_last_of(":");
        if (found == std::string::npos)
            return out;

        out = tmpFnd.substr(found + 1);

        return out;
    }


	std::shared_ptr<PeerConnectionManager> peer_connection_manager() const
	{
		if (peerConnectionManager)
			return peerConnectionManager;
		return nullptr;
	};

	void onConnectionHandler(std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> handler)
	{
		on_connection = handler;
		set_http_handler([this]( websocketpp::connection_hdl hdl)
							   {
								   std::lock_guard<std::mutex> lock(locker);
								   on_connection(this, hdl);
							   });
	}


	void onOpenHandler(std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> openHandler)
	{
		on_open = openHandler;
		set_open_handler([this]( websocketpp::connection_hdl hdl)
							   {
								   std::lock_guard<std::mutex> lock(locker);
								   
								   connection_ptr con = get_con_from_hdl(hdl);
								   std::string remoteEndpoint = con->get_remote_endpoint();
								   remoteEndpoint = parseOfAddress(remoteEndpoint);
								   std::string xforwarded = con->get_request_header("X-Forwarded-For");
								   xforwarded.erase(remove(xforwarded.begin(), xforwarded.end(), ' '), xforwarded.end());
								   std::vector<std::string> proxyes = parseInputString(xforwarded
																						 , ",", true);
								   std::string id;
								   if (proxyes.size() && proxyes[0].size() && proxyes[0] != "::1")
									   id = proxyes[0];
								   else
									   id = remoteEndpoint;
								   


								   websockets.insert(std::pair<std::string, websocketpp::connection_hdl>(id, hdl));
								   on_open(this, hdl);
							   });
	}

	void onCloseHandler(std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> handler)
	{
		on_close = handler;
		set_close_handler([this]( websocketpp::connection_hdl hdl)
								{
									std::lock_guard<std::mutex> lock(locker);
									on_close(this, hdl);
								});
	}

	
	void onMessageHandler(std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl, message_ptr)> i_on_message)
	{
		std::lock_guard<std::mutex> lock(locker);

		on_message = i_on_message;
		set_message_handler([this]( websocketpp::connection_hdl hdl, message_ptr message)
									{
										std::lock_guard<std::mutex> lock(locker);
										
										on_message(this, hdl, message);
									});
	}

	void stop()
	{
		std::lock_guard<std::mutex> lock(locker);
		websocketpp::lib::error_code ec;
		stop_listening(ec);
		if (ec)
		{
			std::cout << "Fail to stop listening : " << ec.message() << std::endl;
			// Failed to stop listening. Log reason using ec.message().
			return;
		}
		std::map<std::string, websocketpp::connection_hdl>::iterator it;
		for (it = websockets.begin(); it != websockets.end(); ++it) {
			websocketpp::lib::error_code ec;
			std::string data = "close";
			close(it->second, websocketpp::close::status::normal, data, ec); // send text message.
			if (ec) { // we got an error
				// Error closing websocket. Log reason using ec.message().
			}
		}

	}
	

public:
	std::map<std::string, websocketpp::connection_hdl> websockets;

	
	
protected:
	std::shared_ptr<PeerConnectionManager> peerConnectionManager;
	std::mutex locker;
};

RTCWebScoketServer* RTCWebScoketServerInit(std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> con_callback,
	std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> open_callback,
	std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl)> cls_callback,
	std::function<void(RTCWebScoketServer*, websocketpp::connection_hdl, message_ptr)> msg_callback,
	std::string basename 
);


