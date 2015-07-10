#ifndef __H264WEBRTC_VIDEO_ENCODER_FACTORY_H__
#define __H264WEBRTC_VIDEO_ENCODER_FACTORY_H__

#include "talk/media/webrtc/webrtcvideoencoderfactory.h"
#include "talk/media/webrtc/webrtcvideodecoderfactory.h"

namespace h264webrtc
{

class H264EncoderFactory : public cricket::WebRtcVideoEncoderFactory
{
public:

    static H264EncoderFactory * Create();

    virtual webrtc::VideoEncoder *CreateVideoEncoder(webrtc::VideoCodecType type);

    virtual void DestroyVideoEncoder(webrtc::VideoEncoder *encoder);

    virtual const std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec> &codecs() const;

private:
    H264EncoderFactory();

    std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec> video_codecs;
};

class H264DecoderFactory : public cricket::WebRtcVideoDecoderFactory
{
public:

    static H264DecoderFactory * Create();

    virtual webrtc::VideoDecoder *CreateVideoDecoder(webrtc::VideoCodecType type);

    virtual void DestroyVideoDecoder(webrtc::VideoDecoder *decoder);

private:
    H264DecoderFactory();

};

}
#endif //__QIC_VIDEO_ENCODER_FACTORY_H__
