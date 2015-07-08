#ifndef __SET_SDP_OBSERVER_H__
#define __SET_SDP_OBSERVER_H__

#include "talk/app/webrtc/peerconnectioninterface.h"

class SetSDPObserver : public webrtc::SetSessionDescriptionObserver {
public:
    static SetSDPObserver* Create();
    virtual void OnSuccess();
    virtual void OnFailure(const std::string& error);

protected:
    SetSDPObserver(){};
};

#endif //__SET_SDP_OBSERVER_H__
