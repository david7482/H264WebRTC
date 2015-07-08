#ifndef __H264WEBRTC_PEER_MANAGER_IMP_H__
#define __H264WEBRTC_PEER_MANAGER_IMP_H__

#include "peer_manager.h"

#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/test/fakeconstraints.h"

namespace h264webrtc
{

class PeerManagerImp : public PeerManager
{
public:
    PeerManagerImp(const std::string &stunurl);

    ~PeerManagerImp();

    void setOffser(const std::string &peerid, const Json::Value &sdp);

    void addIceCandidate(const std::string &peerid, const Json::Value &candidate);

    void deletePeerConnection(const std::string &peerid);

private:
    std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, webrtc::PeerConnectionObserver *> CreatePeerConnection();

    bool AddStreams(webrtc::PeerConnectionInterface *peer_connection);

    cricket::VideoCapturer *OpenVideoCaptureDevice();

    std::string stunurl;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;
    std::map<std::string, rtc::scoped_refptr<webrtc::PeerConnectionInterface> > peer_connection_map;
    std::map<std::string, webrtc::PeerConnectionObserver *> peer_connectionobs_map;
    rtc::Thread *signaling_thread;
    rtc::Thread *worker_thread;
    rtc::scoped_refptr<webrtc::MediaStreamInterface> media_stream;
};

}

#endif //__H264WEBRTC_PEER_MANAGER_IMP_H__
