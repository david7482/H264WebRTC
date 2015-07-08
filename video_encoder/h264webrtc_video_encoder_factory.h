#ifndef __H264WEBRTC_VIDEO_ENCODER_FACTORY_H__
#define __H264WEBRTC_VIDEO_ENCODER_FACTORY_H__

#include "talk/media/webrtc/webrtcvideoencoderfactory.h"

namespace h264webrtc
{

class H264WebrtcVideoEncoderFactory : public cricket::WebRtcVideoEncoderFactory
{
public:

    static H264WebrtcVideoEncoderFactory * Create();

    virtual webrtc::VideoEncoder *CreateVideoEncoder(webrtc::VideoCodecType type);

    virtual void DestroyVideoEncoder(webrtc::VideoEncoder *encoder);

    //virtual bool EncoderTypeHasInternalSource(webrtc::VideoCodecType type) const;

    virtual const std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec> &codecs() const;

private:
    H264WebrtcVideoEncoderFactory();

    std::set<webrtc::VideoCodecType> supported_codec_types;
    std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec> video_codecs;
};

}
#endif //__QIC_VIDEO_ENCODER_FACTORY_H__
