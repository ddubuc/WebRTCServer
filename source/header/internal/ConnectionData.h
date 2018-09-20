#pragma once
#include <memory>
#include <websocketpp/transport/base/connection.hpp>
#include "videorenderer.h"
#include "webrtc.h"


struct ConnectionData
{
	websocketpp::connection_hdl hdl;
	std::unique_ptr<VideoRenderer> renderer;
	std::unique_ptr<PeerConnectionCallback> callback;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc;
	rtc::scoped_refptr<webrtc::MediaStreamInterface> new_stream;
	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track;
	// rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> videoSource;
	// rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;

	Session session;

	ConnectionData(
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> p,
		PeerConnectionCallback* c,
		websocketpp::connection_hdl h
	) : hdl(h), callback(c), pc(p)
	{
	}

	~ConnectionData()
	{
		// do not change the order.!
		
		pc->Close();
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		if (new_stream && video_track)
		{
			new_stream->RemoveTrack(video_track);
			
			pc->RemoveStream(new_stream);
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

		renderer.reset();
		pc = nullptr;
		// Do not change the order.

		LOG(INFO) << "~ConnectionData()";
	}
};
