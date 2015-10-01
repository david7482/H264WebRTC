#include "peer_manager_imp.h"

#include <iostream>

#include "glib.h"
#include "talk/app/webrtc/test/fakeperiodicvideocapturer.h"
#include "talk/app/webrtc/videosourceinterface.h"

#include "codecs/codec_factory.h"
#include "json_parser/json_parser.h"
#include "observers/set_sdp_observer.h"
#include "observers/create_sdp_observer.h"
#include "observers/peer_connection_observer.h"

namespace h264webrtc
{

const std::string AUDIO_LABEL = "audio_label";
const std::string VIDEO_LABEL = "video_label";
const std::string STREAM_LABEL = "stream_label";

// Names used for a IceCandidate JSON object.
const std::string CANDIDATE_SDP_MID_NAME = "sdpMid";
const std::string CANDIDATE_SDP_MLINE_INDEX_NAME = "sdpMLineIndex";
const std::string CANDIDATE_SDP_NAME = "candidate";

// Names used for a SessionDescription JSON object.
const std::string SDP_TYPE_NAME = "type";
const std::string SDP_NAME = "sdp";

class VideoCapturerListener : public sigslot::has_slots<>
{
public:
    VideoCapturerListener(cricket::VideoCapturer *capturer)
    {
        capturer->SignalFrameCaptured.connect(this, &VideoCapturerListener::OnFrameCaptured);
    }

    void OnFrameCaptured(cricket::VideoCapturer *capturer, const cricket::CapturedFrame *frame) {
        g_debug("%s", __FUNCTION__);
    }
};

PeerManager *PeerManager::Create(const std::string &stunurl) {
    return new PeerManagerImp(stunurl);
}

PeerManagerImp::PeerManagerImp(const std::string &stunurl) :
    stunurl(stunurl)
{
    signaling_thread = new rtc::Thread();
    worker_thread = new rtc::Thread();
    signaling_thread->Start();
    worker_thread->Start();

    cricket::WebRtcVideoEncoderFactory *encoder_factory = H264EncoderFactory::Create();
    g_assert(encoder_factory);

    cricket::WebRtcVideoDecoderFactory *decoder_factory = H264DecoderFactory::Create();
    g_assert(decoder_factory);

    peer_connection_factory = webrtc::CreatePeerConnectionFactory(
            worker_thread,
            signaling_thread,
            NULL,
            encoder_factory,
            decoder_factory
    );
    if (!peer_connection_factory.get()) {
        g_critical("Failed to initialize PeerConnectionFactory");
    }
}

PeerManagerImp::~PeerManagerImp()
{
    peer_connection_factory = NULL;
}

void PeerManagerImp::setOffser(const std::string &peerid, const Json::Value &sdp)
{
    g_debug("setOffser -> peerid: %s", peerid.c_str());


    std::string sdp_type, sdp_offer;
    if (!GetStringFromJsonObject(sdp, SDP_TYPE_NAME, &sdp_type) ||
        !GetStringFromJsonObject(sdp, SDP_NAME, &sdp_offer)) {
        g_warning("setOffser <- Can't parse received message.");
        return;
    }

    webrtc::SdpParseError sdp_parse_error;
    webrtc::SessionDescriptionInterface *session_description = webrtc::CreateSessionDescription(sdp_type, sdp_offer, &sdp_parse_error);
    if (!session_description) {
        g_warning("setOffser <- Can't parse received session description message: %s", sdp_parse_error.description.c_str());
        return;
    }
    g_debug("From peerid: %s, sdp sdp_type: %s", peerid.c_str(), session_description->type().c_str());

    std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, webrtc::PeerConnectionObserver *> peer_connection = CreatePeerConnection();
    if (!peer_connection.first) {
        g_warning("setOffser <- Fail to initialize peer connection");
        return;
    }
    g_debug("Success to create peer connection");

    // Set SDP offer to the PeerConnection
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc = peer_connection.first;
    pc->SetRemoteDescription(SetSDPObserver::Create(), session_description);

    // Register this peer
    peer_connection_map.insert(std::pair<std::string, rtc::scoped_refptr<webrtc::PeerConnectionInterface> >(peerid, peer_connection.first));
    peer_connectionobs_map.insert(std::pair<std::string, webrtc::PeerConnectionObserver *>(peerid, peer_connection.second));

    // Create SDP answer
    webrtc::FakeConstraints constraints;
    constraints.SetMandatoryReceiveAudio(false);
    constraints.SetMandatoryReceiveVideo(false);
    pc->CreateAnswer(CreateSDPObserver::Create(pc, signal_sdp_feedback), &constraints);

    g_debug("setOffser <- peerid: %s", peerid.c_str());
}

std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, webrtc::PeerConnectionObserver *> PeerManagerImp::CreatePeerConnection()
{
    webrtc::PeerConnectionInterface::IceServers servers;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "stun:" + stunurl;
    servers.push_back(server);

    PeerConnectionObserver *obs = PeerConnectionObserver::Create(signal_candidate_feedback);
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection =
            peer_connection_factory->CreatePeerConnection(servers, NULL, NULL, NULL, obs);
    if (!peer_connection.get()) {
        g_warning("CreatePeerConnection failed");
        delete obs;
        return std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, webrtc::PeerConnectionObserver *>(
                NULL, NULL);
    }

    AddStreams(peer_connection);

    return std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, webrtc::PeerConnectionObserver *>(
            peer_connection, obs);
}

bool PeerManagerImp::AddStreams(webrtc::PeerConnectionInterface *peer_connection)
{
    if (media_stream.get() == NULL) {

        cricket::VideoCapturer *capturer = OpenVideoCaptureDevice();
        if (!capturer) {
            g_warning("Cannot create capturer");
            return false;
        }

        // Register video capturer listener
        //VideoCapturerListener listener(capturer);

        // Create media stream
        media_stream = peer_connection_factory->CreateLocalMediaStream(STREAM_LABEL);
        if (!media_stream.get()) {
            g_warning("Fail to create stream");
            return false;
        }

        // Create video track
        /*webrtc::FakeConstraints video_constraints;
        video_constraints.SetMandatoryMinWidth(320);
        video_constraints.SetMandatoryMinHeight(480);*/
        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
                peer_connection_factory->CreateVideoTrack(VIDEO_LABEL,
                                                          peer_connection_factory->CreateVideoSource(capturer,
                                                                                                     NULL))
        );

        // Create audio track
        rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
                peer_connection_factory->CreateAudioTrack(AUDIO_LABEL,
                                                          peer_connection_factory->CreateAudioSource(NULL))
        );

        if (!media_stream->AddTrack(video_track)) {
            g_warning("Fail to add video track");
            return false;
        }

        if (!media_stream->AddTrack(audio_track)) {
            g_warning("Fail to add audio track");
            return false;
        }
    }

    if (!peer_connection->AddStream(media_stream)) {
        g_warning("Fail to add media stream to PeerConnection");
        return false;
    }

    return true;

}

cricket::VideoCapturer *PeerManagerImp::OpenVideoCaptureDevice()
{
    cricket::VideoCapturer *capturer = NULL;

    rtc::scoped_ptr<cricket::DeviceManagerInterface> dev_manager(cricket::DeviceManagerFactory::Create());
    if (!dev_manager.get() || !dev_manager->Init()) {
        g_warning("Fail to create device manager");
        return NULL;
    }

    std::vector<cricket::Device> devs;
    if (!dev_manager->GetVideoCaptureDevices(&devs)) {
        g_warning("Fail to enumerate video devices");
        return NULL;
    }

    // Try to use the normal camera device
    for (std::vector<cricket::Device>::iterator it = devs.begin(); it != devs.end(); ++it) {
        cricket::Device device = *it;
        capturer = dev_manager->CreateVideoCapturer(device);
        if (capturer) {
            break;
        } else {
            g_warning("Fail to create video capture device: %s", it->id.c_str());
        }
    }

    // Use the fake yuvframegenerator
    if (capturer == NULL) {
        cricket::Device device;
        if (dev_manager->GetVideoCaptureDevice("YuvFramesGenerator", &device)) {
            capturer = dev_manager->CreateVideoCapturer(device);
        }
        if (capturer == NULL) {
            g_warning("Fail to get fake video devices");
        } else {
            g_warning("Success to create fake video devices");
        }
    }

    if (capturer != NULL)
        g_debug("Create capturer device: %s", capturer->GetId().c_str());

    return capturer;
}

void PeerManagerImp::deletePeerConnection(const std::string &peerid)
{
    std::map<std::string, rtc::scoped_refptr<webrtc::PeerConnectionInterface> >::iterator it = peer_connection_map.find(
            peerid);
    if (it != peer_connection_map.end()) {
        it->second = NULL;
        peer_connection_map.erase(it);
    }

    std::map<std::string, webrtc::PeerConnectionObserver *>::iterator obs_it = peer_connectionobs_map.find(peerid);
    if (obs_it == peer_connectionobs_map.end()) {
        PeerConnectionObserver *obs = dynamic_cast<PeerConnectionObserver *>(obs_it->second);
        delete obs;
        peer_connectionobs_map.erase(obs_it);
    }
}

void PeerManagerImp::addIceCandidate(const std::string &peerid, const Json::Value &candidate)
{
    g_debug("addIceCandidate -> peerid: %s", peerid.c_str());

    std::map<std::string, rtc::scoped_refptr<webrtc::PeerConnectionInterface> >::iterator it = peer_connection_map.find(peerid);
    if (it == peer_connection_map.end()) {
        g_warning("addIceCandidate <- Fail to find the existed peer connection.");
        return;
    }

    std::string sdp_mid, sdp;
    int sdp_mlineindex = 0;
    if (!GetStringFromJsonObject(candidate, CANDIDATE_SDP_MID_NAME, &sdp_mid) ||
        !GetIntFromJsonObject(candidate, CANDIDATE_SDP_MLINE_INDEX_NAME, &sdp_mlineindex) ||
        !GetStringFromJsonObject(candidate, CANDIDATE_SDP_NAME, &sdp)) {
        g_warning("addIceCandidate <- Fail to parse received message.");
        return;
    }

    rtc::scoped_ptr<webrtc::IceCandidateInterface> ice_candidate(webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp));
    if (!ice_candidate.get()) {
        g_warning("addIceCandidate <- Fail to parse received candidate message.");
        return;
    }

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc = it->second;
    if (!pc->AddIceCandidate(ice_candidate.get())) {
        g_warning("addIceCandidate <- Failed to apply the received candidate");
        return;
    }

    g_debug("addIceCandidate <- peerid: %s", peerid.c_str());
}

}