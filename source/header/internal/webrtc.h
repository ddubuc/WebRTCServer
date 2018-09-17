#ifndef OMEKASHI_WEBRTC_H
#define OMEKASHI_WEBRTC_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/base/json.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/ssladapter.h"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class DummySetSessionDescriptionObserver
: public webrtc::SetSessionDescriptionObserver {
public:
   static DummySetSessionDescriptionObserver* Create() {
		return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
	}
	virtual void OnSuccess() {
		LOG(INFO) << __FUNCTION__;
	}
	virtual void OnFailure(const std::string& error) {
		LOG(INFO) << __FUNCTION__ << " " << error;
	}

	protected:
	DummySetSessionDescriptionObserver() {}
	~DummySetSessionDescriptionObserver() {}
};

class PeerConnectionCallback : public webrtc::PeerConnectionObserver{
	private:
		std::function<void(rtc::scoped_refptr<webrtc::MediaStreamInterface>)> onAddStream;
		std::function<void(const webrtc::IceCandidateInterface*)> onIceCandidate;
		std::function<void()> onNegociation;
		std::function<void(webrtc::PeerConnectionInterface::SignalingState)> onSignalingChange;
	public:
		PeerConnectionCallback() 
		: onAddStream(nullptr), onIceCandidate(nullptr), onNegociation(nullptr), onSignalingChange(nullptr) {
		}
		virtual ~PeerConnectionCallback() {}
    void SetOnAddStream(std::function<void(rtc::scoped_refptr<webrtc::MediaStreamInterface>)> callback) {
      onAddStream = callback;
    }
    void SetOnIceCandidate(std::function<void(const webrtc::IceCandidateInterface*)> callback) {
      onIceCandidate = callback;
    }

	void SetOnNegotiation(std::function<void()> callback)
	{
		onNegociation = callback;
	}

	void SetOnSignalingChange(std::function<void(webrtc::PeerConnectionInterface::SignalingState)> callback)
	{
		onSignalingChange = callback;
	}

protected:
  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override{
	  LOG(INFO) << __FUNCTION__ << " " << new_state;
	  if (onSignalingChange) onSignalingChange(new_state);
  }
  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
  void OnRenegotiationNeeded() override {
	  LOG(INFO) << __FUNCTION__ << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1";
	  if (onNegociation) onNegociation();
  }
  void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override{
	  LOG(INFO) << __FUNCTION__ << " " << new_state;
  };
  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override{
	  LOG(INFO) << __FUNCTION__ << " " << new_state;
  };
  void OnIceConnectionReceivingChange(bool receiving) override {
	  LOG(INFO) << __FUNCTION__ << " " << receiving;
  }

  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {
    onIceCandidate(candidate);
  };
  void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {
	  LOG(INFO) << __FUNCTION__ << " " << stream->label();
  };
  void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {
    LOG(INFO) << __FUNCTION__ << " " << stream->label();
    onAddStream(stream);
  };

};

class CreateSDPCallback : public webrtc::CreateSessionDescriptionObserver {
	private:
		std::function<void(webrtc::SessionDescriptionInterface*)> success;
		std::function<void(const std::string&)> failure;
	public:
		CreateSDPCallback(std::function<void(webrtc::SessionDescriptionInterface*)> s, std::function<void(const std::string&)> f)
      : success(s), failure(f) {
		};
		void OnSuccess(webrtc::SessionDescriptionInterface* desc) {
      LOG(INFO) << __FUNCTION__ ;
			if (success) {
				success(desc);
			}
		}
		void OnFailure(const std::string& error) {
      LOG(INFO) << __FUNCTION__ ;
			if (failure) {
				failure(error);
			} else {
				LOG(LERROR) << error;
			}
		}
};

inline rtc::scoped_refptr<webrtc::PeerConnectionInterface> CreatePeerConnection(webrtc::PeerConnectionObserver* observer) {
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory
    = webrtc::CreatePeerConnectionFactory();
	if (!peer_connection_factory.get()) {
		LOG(LERROR) << "Failed to initialize PeerConnectionFactory" << std::endl;
	  return nullptr;
	}

  webrtc::FakeConstraints constraints;
  constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "true");

	webrtc::PeerConnectionInterface::RTCConfiguration config;
	webrtc::PeerConnectionInterface::IceServer server0;
	webrtc::PeerConnectionInterface::IceServer server1;
	webrtc::PeerConnectionInterface::IceServer server2;
	webrtc::PeerConnectionInterface::IceServer server3;
	webrtc::PeerConnectionInterface::IceServer server4;
  
	server0.uri = "stun:stun.l.google.com:19302";
	server1.uri = "stun:stun1.l.google.com:19302";
	server2.uri = "stun:stun2.l.google.com:19302";
	server3.uri = "stun:stun3.l.google.com:19302";
	server4.uri = "stun:stun4.l.google.com:19302";

	config.servers.push_back(server0);
	config.servers.push_back(server1);
	config.servers.push_back(server2);
	config.servers.push_back(server3);
	config.servers.push_back(server4);


  return peer_connection_factory->CreatePeerConnection(
      config, &constraints, NULL, NULL, observer);
}

inline void ProcessMessage() {
  auto thread = rtc::Thread::Current();
  auto msg_cnt = thread->size();
  LOG(INFO) << "process message. : last " << msg_cnt;

  while(msg_cnt > 0) {
    rtc::Message msg;
    if (!thread->Get(&msg, 0))
      return;
    thread->Dispatch(&msg);
  }
}



#endif // OMEKASHI_WEBRTC_H
