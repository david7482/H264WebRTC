# H264WebRTC
This project try to integrate OpenH264 as the H264 implementation into Google WebRTC. It takes [Ericsson's signaling server](http://demo.openwebrtc.io:38080/) as the signaling server.

## Dependency
* glib-2.0
* libsoup-2.4
* x11
* sigc++-2.0
* jsoncpp
* [openh264](https://github.com/cisco/openh264)

## How to build
* Build [webrtc](https://github.com/david7482/webrtcbuilds-builder) first
  * It will install the build at /opt/webrtc
* We use [Scons](http://www.scons.org/) as the build system
* Run `scons` then it will build the executable at `bin/`
 
## How to run
* [Setup Firefox](https://github.com/EricssonResearch/openwebrtc/issues/425#issuecomment-119020464) to use H264 in a higher priority
* Start a session from [here](http://demo.openwebrtc.io:38080/)
* Run `./H264WebRTC` with `-s [session id]`
  * You could also try `-v` as the verbose mode
* You should get the stream on Firefox.
* Check SDP part of `about:webrtc` in Firefox to make sure if it is running on H264 
