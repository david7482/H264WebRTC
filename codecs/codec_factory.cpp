#include "codec_factory.h"
#include "glib.h"

#include "webrtc/modules/video_coding/codecs/vp8/include/vp8.h"
#include "codecs/h264/h264_decoder.h"
#include "codecs/h264/h264_encoder.h"

namespace h264webrtc
{

H264EncoderFactory *H264EncoderFactory::Create()
{
    return new H264EncoderFactory();
}

H264EncoderFactory::H264EncoderFactory()
{
    // H264
    video_codecs.push_back(
            cricket::WebRtcVideoEncoderFactory::VideoCodec(webrtc::kVideoCodecH264, "H264", 1920, 1080, 30));

    // VP8
    video_codecs.push_back(
            cricket::WebRtcVideoEncoderFactory::VideoCodec(webrtc::kVideoCodecVP8, "VP8", 1920, 1080, 30));
}

webrtc::VideoEncoder *H264EncoderFactory::CreateVideoEncoder(webrtc::VideoCodecType type)
{
    g_debug("%s -> type: %d", __FUNCTION__, type);

    webrtc::VideoEncoder *encoder = NULL;
    switch (type) {
        case webrtc::kVideoCodecVP8:
            g_debug("Create VP8 encoder ->", __FUNCTION__);
            encoder = webrtc::VP8Encoder::Create();
            g_debug("Create VP8 encoder <-", __FUNCTION__);
            break;
        case webrtc::kVideoCodecH264:
            g_debug("Create H264 encoder ->", __FUNCTION__);
            encoder = H264Encoder::Create();
            g_debug("Create H264 encoder ->", __FUNCTION__);
            break;
        default:
            g_debug("%s <- invalid codec type", __FUNCTION__);
            break;
    }

    g_debug("%s <- encoder = %p", __FUNCTION__, encoder);
    return encoder;
}

void H264EncoderFactory::DestroyVideoEncoder(webrtc::VideoEncoder *encoder)
{
    g_debug("%s ->", __FUNCTION__);
    delete encoder;
    g_debug("%s <-", __FUNCTION__);
}

const std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec> &H264EncoderFactory::codecs() const
{
    return video_codecs;
}

H264DecoderFactory *H264DecoderFactory::Create()
{
    return new H264DecoderFactory();
}

H264DecoderFactory::H264DecoderFactory()
{
}

webrtc::VideoDecoder *H264DecoderFactory::CreateVideoDecoder(webrtc::VideoCodecType type)
{
    g_debug("%s -> type: %d", __FUNCTION__, type);

    webrtc::VideoDecoder *decoder = NULL;
    switch (type) {
        case webrtc::kVideoCodecVP8:
            g_debug("Create VP8 decoder ->", __FUNCTION__);
            decoder = webrtc::VP8Decoder::Create();
            g_debug("Create VP8 decoder <-", __FUNCTION__);
            break;
        case webrtc::kVideoCodecH264:
            g_debug("Create H264 decoder ->", __FUNCTION__);
            decoder = H264Decoder::Create();
            g_debug("Create H264 decoder ->", __FUNCTION__);
            break;
        default:
            g_debug("%s <- invalid codec type", __FUNCTION__);
            break;
    }

    g_debug("%s <- decoder = %p", __FUNCTION__, decoder);
    return decoder;
}

void H264DecoderFactory::DestroyVideoDecoder(webrtc::VideoDecoder *decoder)
{
    g_debug("%s ->", __FUNCTION__);
    delete decoder;
    g_debug("%s <-", __FUNCTION__);
}

}