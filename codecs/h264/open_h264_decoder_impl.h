#ifndef __OPEN_H264_DECODER_IMPL_H__
#define __OPEN_H264_DECODER_IMPL_H__

#include "h264_decoder.h"

namespace h264webrtc
{

class OpenH264DecoderImpl : public H264Decoder
{
public:
    OpenH264DecoderImpl();

    virtual int InitDecode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores);

    virtual int Decode(const webrtc::EncodedImage& input_image,
                           bool missing_frames,
                           const webrtc::RTPFragmentationHeader* fragmentation,
                           const webrtc::CodecSpecificInfo* codec_specific_info = NULL,
                           int64_t render_time_ms = -1);

    virtual int RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback);

    virtual int Release();
    virtual int Reset();

    virtual ~OpenH264DecoderImpl();
};

}// namespace h264webrtc

#endif //__OPEN_H264_DECODER_IMPL_H__
