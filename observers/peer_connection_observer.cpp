#include "peer_connection_observer.h"

#include "glib.h"

const std::string CANDIDATE_SDP_MID_NAME = "sdpMid";
const std::string CANDIDATE_SDP_MLINE_INDEX_NAME = "sdpMLineIndex";
const std::string CANDIDATE_SDP_NAME = "candidate";

PeerConnectionObserver *PeerConnectionObserver::Create() {
    return new PeerConnectionObserver();
}

void PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
    g_debug("OnIceCandidate -> mline_index: %d", candidate->sdp_mline_index());

    Json::Value jmessage;
    std::string candidate_string;
    jmessage[CANDIDATE_SDP_MID_NAME] = candidate->sdp_mid();
    jmessage[CANDIDATE_SDP_MLINE_INDEX_NAME] = candidate->sdp_mline_index();
    if (!candidate->ToString(&candidate_string)) {
        g_warning("Failed to serialize candidate");
        return;
    }
    jmessage[CANDIDATE_SDP_NAME] = candidate_string;

    g_debug("OnIceCandidate <-");
}