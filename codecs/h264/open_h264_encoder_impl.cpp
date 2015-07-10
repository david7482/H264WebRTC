#include "open_h264_encoder_impl.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <wels/codec_app_def.h>

#include "glib.h"
#include "webrtc/common.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/modules/interface/module_common_types.h"
#include "webrtc/system_wrappers/interface/trace.h"
#include "webrtc/system_wrappers/interface/tick_util.h"
#include "webrtc/system_wrappers/interface/trace_event.h"
#include "webrtc/base/scoped_ptr.h"
#include "wels/codec_app_def.h"

namespace h264webrtc
{


H264Encoder *H264Encoder::Create()
{
    return new OpenH264EncoderImpl();
}

OpenH264EncoderImpl::OpenH264EncoderImpl() :
        buffer(NULL),
        buffer_size(0),
        encoded_complete_callback(NULL),
        inited(false),
        timestamp(0),
        encoder(NULL)
{
    memset(&codec, 0, sizeof(codec));
    uint32_t seed = static_cast<uint32_t>(webrtc::TickTime::MillisecondTimestamp());
    srand(seed);
}

OpenH264EncoderImpl::~OpenH264EncoderImpl()
{
    Release();
}

int OpenH264EncoderImpl::Release()
{
    if (buffer != NULL) {
        delete[] buffer;
        buffer = NULL;
    }
    if (encoder != NULL) {
        WelsDestroySVCEncoder(encoder);
        encoder = NULL;
    }
    buffer_size = 0;
    inited = false;
    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264EncoderImpl::SetRates(uint32_t new_bitrate_kbit, uint32_t new_framerate)
{
    g_debug("%s -> (%d, %d)", __FUNCTION__, new_bitrate_kbit, new_framerate);

    if (!inited) {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    if (new_framerate < 1) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    // update bit rate
    if (codec.maxBitrate > 0 && new_bitrate_kbit > codec.maxBitrate) {
        new_bitrate_kbit = codec.maxBitrate;
    }

    g_debug("%s <-", __FUNCTION__);
    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264EncoderImpl::InitEncode(const webrtc::VideoCodec *codec_settings, int number_of_cores, size_t max_payload_size)
{
    g_debug("%s: codec_settings(%d x %d : %d, StartBitrate: %d kbps)",
             __FUNCTION__,
             codec_settings->width, codec_settings->height,
             codec_settings->maxFramerate,
             codec_settings->startBitrate);

    if (codec_settings->maxFramerate < 1) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    // allow zero to represent an unspecified maxBitRate
    if (codec_settings->maxBitrate > 0 && codec_settings->startBitrate > codec_settings->maxBitrate) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (codec_settings->width < 1 || codec_settings->height < 1) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (number_of_cores < 1) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }

    int ret_val = Release();
    if (ret_val < 0) {
        return ret_val;
    }
    if (encoder == NULL) {
        ret_val = WelsCreateSVCEncoder(&encoder);
        if (ret_val != 0) {
            g_warning("%s: fails to create encoder: %d", __FUNCTION__, ret_val);
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
    }

    SEncParamBase param;
    memset(&param, 0, sizeof(SEncParamBase));
    param.iUsageType = CAMERA_VIDEO_REAL_TIME;
    param.iRCMode = RC_QUALITY_MODE;
    param.fMaxFrameRate = codec_settings->maxFramerate;
    param.iPicWidth = codec_settings->width;
    param.iPicHeight = codec_settings->height;
    param.iTargetBitrate = codec_settings->startBitrate * 1024;
    ret_val = encoder->Initialize(&param);
    if (ret_val != 0) {
        g_warning("%s: fails to initialize encoder: %d", __FUNCTION__, ret_val);
        WelsDestroySVCEncoder(encoder);
        encoder = NULL;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    if (&codec != codec_settings) {
        codec = *codec_settings;
    }

    if (buffer != NULL) {
        delete[] buffer;
    }
    buffer_size = webrtc::CalcBufferSize(webrtc::kI420, codec.width, codec.height);
    buffer = new uint8_t[buffer_size];

    timestamp = 0;
    inited = true;

    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264EncoderImpl::GetTotalNaluCount(const SFrameBSInfo &info)
{
    if (info.iLayerNum <= 0 || info.iFrameSizeInBytes <= 0) {
        return 0;
    }

    uint32_t totalNaluCount = 0;

    for (int layer_index = 0; layer_index < info.iLayerNum; layer_index++) {
        const SLayerBSInfo *layer_bs_info = &info.sLayerInfo[layer_index];
        totalNaluCount += layer_bs_info->iNalCount;
    }

    return totalNaluCount;
}

bool OpenH264EncoderImpl::GetRTPFragmentationHeaderH264(const SFrameBSInfo &info, webrtc::RTPFragmentationHeader &header, webrtc::EncodedImage &encoded_image)
{
    uint32_t totalNaluIndex = 0;
    for (int layer = 0; layer < info.iLayerNum; layer++) {

        const SLayerBSInfo *layer_bs_info = &info.sLayerInfo[layer];
        if (layer_bs_info != NULL) {

            int layer_size = 0;
            int nal_begin = 4;
            uint8_t *nal_buffer = NULL;

            for (int nal_index = 0; nal_index < layer_bs_info->iNalCount; nal_index++) {

                nal_buffer = layer_bs_info->pBsBuf + nal_begin;
                layer_size += layer_bs_info->pNalLengthInByte[nal_index];
                nal_begin += layer_size;

                int currentNaluSize = layer_bs_info->pNalLengthInByte[nal_index] - 4;
                memcpy(encoded_image._buffer + encoded_image._length, nal_buffer, (size_t)currentNaluSize);
                encoded_image._length += currentNaluSize;

                header.fragmentationOffset[totalNaluIndex] = encoded_image._length - currentNaluSize;
                header.fragmentationLength[totalNaluIndex] = (size_t)currentNaluSize;
                header.fragmentationPlType[totalNaluIndex] = 0;
                header.fragmentationTimeDiff[totalNaluIndex] = 0;
                totalNaluIndex++;
            }
        }
    }

    return true;
}

int OpenH264EncoderImpl::Encode(const webrtc::VideoFrame &input_image,
                                const webrtc::CodecSpecificInfo *codec_specific_info,
                                const std::vector<webrtc::VideoFrameType> *frame_types)
{
    if (!inited) {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    if (input_image.IsZeroSize()) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (encoded_complete_callback == NULL) {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }

    // We only support one stream at the moment.
    webrtc::VideoFrameType frame_type = webrtc::kDeltaFrame;
    if (frame_types && frame_types->size() > 0) {
        frame_type = (*frame_types)[0];
    }
    g_debug("%s -> frame_type: %d, RTS: %ld, TS: %ld", __FUNCTION__, frame_type, input_image.render_time_ms(), timestamp);

    bool send_keyframe = (frame_type == webrtc::kKeyFrame);
    if (send_keyframe) {
        encoder->ForceIntraFrame(true);
        g_debug("ForceIntraFrame(width:%d, height:%d)", input_image.width(), input_image.height());
    }

    // Check for change in frame size.
    if (input_image.width() != codec.width || input_image.height() != codec.height) {
        codec.width  = (unsigned short) input_image.width();
        codec.height = (unsigned short) input_image.height();
    }

    SSourcePicture pic = {0};
    pic.iPicWidth = input_image.width();
    pic.iPicHeight = input_image.height();
    pic.iColorFormat = videoFormatI420;
    pic.iStride[0] = input_image.stride(webrtc::kYPlane);
    pic.iStride[1] = input_image.stride(webrtc::kUPlane);
    pic.iStride[2] = input_image.stride(webrtc::kVPlane);
    pic.pData[0] = const_cast<uint8_t *>(input_image.buffer(webrtc::kYPlane));
    pic.pData[1] = const_cast<uint8_t *>(input_image.buffer(webrtc::kUPlane));
    pic.pData[2] = const_cast<uint8_t *>(input_image.buffer(webrtc::kVPlane));
    // Cheat OpenH264 for the framerate
    pic.uiTimeStamp = timestamp;
    timestamp += (int64_t)(1000 / codec.maxFramerate);

    SFrameBSInfo info = {0};
    int retVal = encoder->EncodeFrame(&pic, &info);
    if (retVal == videoFrameTypeSkip ||
        info.iLayerNum == 0 ||
        info.iFrameSizeInBytes == 0) {
        g_debug("%s <- Skip frame, retVal: %d\n", __FUNCTION__, retVal);
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int totalNaluCount = GetTotalNaluCount(info);
    if (totalNaluCount == 0) {
        g_debug("%s <- No NALU\n", __FUNCTION__);
        return WEBRTC_VIDEO_CODEC_OK;
    }

    g_debug("%s layerNum: %d, FrameSizeInBytes: %d, NALUCount: %d", __FUNCTION__, info.iLayerNum, info.iFrameSizeInBytes, totalNaluCount);

    webrtc::EncodedImage encoded_image(buffer, 0, buffer_size);
    webrtc::RTPFragmentationHeader header;
    header.VerifyAndAllocateFragmentationHeader((size_t)totalNaluCount);
    GetRTPFragmentationHeaderH264(info, header, encoded_image);

    webrtc::CodecSpecificInfo codec_info;
    memset(&codec_info, 0, sizeof(codec_info));
    codec_info.codecType = webrtc::kVideoCodecH264;

    encoded_image._encodedWidth = codec.width;
    encoded_image._encodedHeight = codec.height;
    encoded_image._timeStamp = input_image.timestamp();
    encoded_image.capture_time_ms_ = input_image.render_time_ms();
    encoded_image._frameType = frame_type;
    encoded_image._completeFrame = true;

    // call back
    encoded_complete_callback->Encoded(encoded_image, &codec_info, &header);

    g_debug("%s <- \n", __FUNCTION__);
    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264EncoderImpl::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback *callback)
{
    encoded_complete_callback = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int OpenH264EncoderImpl::SetChannelParameters(uint32_t /*packet_loss*/, int64_t rtt)
{
    return WEBRTC_VIDEO_CODEC_OK;
}

}// namespace h264webrtc