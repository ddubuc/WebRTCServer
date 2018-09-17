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
	rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> videoSource;

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
		

		if (new_stream && video_track)
		{
			new_stream->RemoveTrack(video_track);
			
			pc->RemoveStream(new_stream);
		}
		pc->Close();
		renderer.reset();
		pc = nullptr;
		// Do not change the order.

		LOG(INFO) << "~ConnectionData()";
	}
};
