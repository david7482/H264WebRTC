#ifndef __CREATE_SDP_OBSERVER_H__
#define __CREATE_SDP_OBSERVER_H__

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "peer_manager.h"

class CreateSDPObserver : public webrtc::CreateSessionDescriptionObserver {
public:
    static CreateSDPObserver * Create(webrtc::PeerConnectionInterface* pc, sigc::signal<void, Json::Value> signal_sdp_feedback);

    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
    virtual void OnFailure(const std::string& error);

protected:
    CreateSDPObserver(webrtc::PeerConnectionInterface* pc, sigc::signal<void, Json::Value> signal_sdp_feedback)
        : m_pc(pc),
          signal_sdp_feedback(signal_sdp_feedback)
    {};

private:
    webrtc::PeerConnectionInterface* m_pc;
    sigc::signal<void, Json::Value> signal_sdp_feedback;
};

#endif //__CREATE_SDP_OBSERVER_H__
