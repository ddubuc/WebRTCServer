#pragma once
#include <memory>
#include <websocketpp/transport/base/connection.hpp>
#include "videorenderer.h"
#include "webrtc.h"


struct ConnectionData
{
	websocketpp::connection_hdl hdl;
	std::unique_ptr<VideoRenderer> renderer;
	PeerConnectionCallback * callback;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface>  pc;
	// rtc::scoped_refptr<webrtc::MediaStreamInterface> new_stream;
	// rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track;

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
		
		//pc->Close();
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		rtc::scoped_refptr<webrtc::StreamCollectionInterface> localstreams (pc->local_streams());
		for (unsigned int i = 0; i<localstreams->count(); i++)
		{
			auto stream = localstreams->at(i);
			pc->RemoveStream(stream);
		}
		pc->Close();
		
		// if (new_stream && video_track)
		// {
		// 	new_stream->RemoveTrack(video_track);
			
		// 	pc->RemoveStream(new_stream);
		// }
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

		renderer.reset();
		pc = nullptr;
		// Do not change the order.

		LOG(INFO) << "~ConnectionData()";
	}
};
