#ifndef __H264_ENCODER_H__
#define __H264_ENCODER_H__

#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"

namespace h264webrtc
{

class H264Encoder : public webrtc::VideoEncoder
{
public:
    static H264Encoder *Create();

    virtual ~H264Encoder() { };
};

}// namespace h264webrtc

#endif // __H264_ENCODER_H__
