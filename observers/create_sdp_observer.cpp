#include "create_sdp_observer.h"

#include "glib.h"

#include "set_sdp_observer.h"
#include "peer_manager.h"

const std::string SDP_TYPE_NAME = "type";
const std::string SDP_NAME = "sdp";

CreateSDPObserver *CreateSDPObserver::Create(webrtc::PeerConnectionInterface* pc,
                                             sigc::signal<void, Json::Value> signal_sdp_feedback)
{
    return  new rtc::RefCountedObject<CreateSDPObserver>(pc, signal_sdp_feedback);
}

void CreateSDPObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
    m_pc->SetLocalDescription(SetSDPObserver::Create(), desc);

    Json::Value jmessage;
    jmessage[SDP_TYPE_NAME] = desc->type();
    std::string sdp;
    if (!desc->ToString(&sdp)) {
        g_warning("Failed to serialize sdp");
        return;
    }

    // trick for Firefox H264 support
    sdp += "a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1\r\n";
    jmessage[SDP_NAME] = sdp;

    signal_sdp_feedback(jmessage);
}

void CreateSDPObserver::OnFailure(const std::string& error)
{
    g_warning("Fail to create SDP: %s", error.c_str());
}