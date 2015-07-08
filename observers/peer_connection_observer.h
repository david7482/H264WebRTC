#ifndef __PEER_CONNECTION_OBSERVER_H__
#define __PEER_CONNECTION_OBSERVER_H__

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/mediastreaminterface.h"

#include "peer_manager.h"

class PeerConnectionObserver : public webrtc::PeerConnectionObserver
{
public:
    static PeerConnectionObserver * Create();
    virtual ~PeerConnectionObserver() {}

    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
    virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {}
    virtual void OnStateChange(webrtc::PeerConnectionObserver::StateType state_changed) {}
    virtual void OnAddStream(webrtc::MediaStreamInterface* stream) {}
    virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream) {}
    virtual void OnDataChannel(webrtc::DataChannelInterface* channel) {}
    virtual void OnRenegotiationNeeded() {}
    virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {}
    virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {}
    virtual void OnIceComplete() {}

protected:
    PeerConnectionObserver() {};

};

#endif //__PEER_CONNECTION_OBSERVER_H__
