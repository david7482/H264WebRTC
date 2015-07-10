#ifndef __H264_DECODER_H__
#define __H264_DECODER_H__

#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"

namespace h264webrtc
{

class H264Decoder : public webrtc::VideoDecoder {
public:
    static H264Decoder * Create();

    virtual ~H264Decoder() {};
};

}// namespace h264webrtc

#endif //__H264_DECODER_H__
