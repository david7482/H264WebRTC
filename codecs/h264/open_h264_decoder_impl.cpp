#include "open_h264_decoder_impl.h"

namespace h264webrtc
{


H264Decoder *H264Decoder::Create()
{
    return new OpenH264DecoderImpl();
}

OpenH264DecoderImpl::OpenH264DecoderImpl()
{

}

OpenH264DecoderImpl::~OpenH264DecoderImpl()
{

}

int OpenH264DecoderImpl::InitDecode(const webrtc::VideoCodec* codec_settings, int32_t number_of_cores)
{
    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264DecoderImpl::Decode(const webrtc::EncodedImage& input_image,
                       bool missing_frames,
                       const webrtc::RTPFragmentationHeader* fragmentation,
                       const webrtc::CodecSpecificInfo* codec_specific_info,
                       int64_t render_time_ms)
{
    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264DecoderImpl::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback)
{
    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264DecoderImpl::Release()
{
    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264DecoderImpl::Reset()
{
    return WEBRTC_VIDEO_CODEC_OK;
}

}// namespace h264webrtc