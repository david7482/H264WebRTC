#ifndef __CREATE_SDP_OBSERVER_H__
#define __CREATE_SDP_OBSERVER_H__

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "peer_manager.h"

class CreateSDPObserver : public webrtc::CreateSessionDescriptionObserver {
public:
    static CreateSDPObserver * Create(webrtc::PeerConnectionInterface* pc);

    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
    virtual void OnFailure(const std::string& error);

protected:
    CreateSDPObserver(webrtc::PeerConnectionInterface* pc) :
            m_pc(pc) {};

private:
    webrtc::PeerConnectionInterface* m_pc;
};

#endif //__CREATE_SDP_OBSERVER_H__
