#include "h264webrtc_video_encoder_factory.h"

#include "glib.h"
#include "webrtc/modules/video_coding/codecs/vp8/include/vp8.h"

#include "video_encoder/h264/h264_video_encoder.h"

namespace h264webrtc
{

H264WebrtcVideoEncoderFactory *H264WebrtcVideoEncoderFactory::Create()
{
    return new H264WebrtcVideoEncoderFactory();
}

H264WebrtcVideoEncoderFactory::H264WebrtcVideoEncoderFactory()
{
    // H264
    supported_codec_types.insert(webrtc::kVideoCodecH264);
    video_codecs.push_back(
            cricket::WebRtcVideoEncoderFactory::VideoCodec(webrtc::kVideoCodecH264, "H264", 1920, 1080, 30));

    // VP8
    supported_codec_types.insert(webrtc::kVideoCodecVP8);
    video_codecs.push_back(
            cricket::WebRtcVideoEncoderFactory::VideoCodec(webrtc::kVideoCodecVP8, "VP8", 1920, 1080, 30));
}

webrtc::VideoEncoder *H264WebrtcVideoEncoderFactory::CreateVideoEncoder(webrtc::VideoCodecType type)
{
    g_debug("%s -> type: %d", __FUNCTION__, type);

    if (supported_codec_types.count(type) == 0) {
        g_debug("%s <- encoder = NULL", __FUNCTION__);
        return NULL;
    }

    webrtc::VideoEncoder *encoder = NULL;
    switch (type) {
        case webrtc::kVideoCodecVP8:
            g_debug("Create VP8 encoder ->", __FUNCTION__);
            encoder = webrtc::VP8Encoder::Create();
            g_debug("Create VP8 encoder <-", __FUNCTION__);
            break;
        case webrtc::kVideoCodecH264:
            g_debug("Create H264 encoder ->", __FUNCTION__);
            encoder = OpenH264Encoder::Create();
            g_debug("Create H264 encoder ->", __FUNCTION__);
            break;
        default:
            break;
    }

    g_debug("%s <- encoder = %p", __FUNCTION__, encoder);
    return encoder;
}

/*bool H264WebrtcVideoEncoderFactory::EncoderTypeHasInternalSource(webrtc::VideoCodecType type) const
{
    g_debug("%s -> type: %d", __FUNCTION__, type);

    bool ret = false;
    switch (type) {
        case webrtc::kVideoCodecVP8:
            //ret =  true;
            break;
        case webrtc::kVideoCodecH264:
            //ret =  true;
            break;
        default:
            ret = false;
            break;
    }

    g_debug("%s <- type: %d, ret = %d", __FUNCTION__, type, ret);
    return ret;
}*/

void H264WebrtcVideoEncoderFactory::DestroyVideoEncoder(webrtc::VideoEncoder *encoder)
{
    g_debug("%s ->", __FUNCTION__);
    delete encoder;
    g_debug("%s <-", __FUNCTION__);
}

const std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec> &H264WebrtcVideoEncoderFactory::codecs() const
{
    return video_codecs;
}

}