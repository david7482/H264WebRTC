#ifndef __H264WEBRTC_PEER_MANAGER_H__
#define __H264WEBRTC_PEER_MANAGER_H__

#include <string>
#include <sigc++/signal.h>

#include "json/json.h"

namespace h264webrtc
{

class PeerManager
{
public:
    static PeerManager *Create(const std::string &stunurl);
    virtual ~PeerManager() {};

    virtual void setOffser(const std::string &peerid, const Json::Value &sdp) = 0;
    virtual void addIceCandidate(const std::string &peerid, const Json::Value &candidate) = 0;
    virtual void deletePeerConnection(const std::string &peerid) = 0;

    sigc::signal<void, Json::Value> signal_sdp_feedback;
    sigc::signal<void, Json::Value> signal_candidate_feedback;
};

}
#endif // __H264WEBRTC_PEER_MANAGER_H__
