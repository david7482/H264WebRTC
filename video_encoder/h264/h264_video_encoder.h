#ifndef __OPEN_H264_VIDEO_ENCODER_H__
#define __OPEN_H264_VIDEO_ENCODER_H__

#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"

class OpenH264Encoder : public webrtc::VideoEncoder {
 public:
  static OpenH264Encoder * Create();

  virtual ~OpenH264Encoder() {};
};  // end of OpenH264Encoder class

#endif // __OPEN_H264_VIDEO_ENCODER_H__
