import os

# Set our required libraries
libraries 		= []
library_paths 	= ''
cppDefines 		= {}
cFlags 		    = ['-Wall']

cFlags += ['-g', '-O0']

# Define the attributes of the build environment shared
env = Environment(CPPPATH = ['.'])
env.Append(LIBS 		= libraries)
env.Append(LIBPATH 		= library_paths)
env.Append(CPPDEFINES 	= cppDefines)
env.Append(CFLAGS 		= cFlags)
env.Append(CPPFLAGS 	= cFlags)

GWR_PKG_ROOT_PATH = '/home/david74/projects/vsimon-webrtc-builder-test/out/webrtcbuilds-9468-1adbacb-linux64'
env['ENV']['PKG_CONFIG_PATH'] = '%s/lib/Release/pkgconfig' % GWR_PKG_ROOT_PATH

env.ParseConfig('pkg-config --cflags --libs glib-2.0')
env.ParseConfig('pkg-config --cflags --libs libsoup-2.4')
env.ParseConfig('pkg-config --cflags --libs uuid')
env.ParseConfig('pkg-config --cflags --libs libwebrtc_full')
env.ParseConfig('pkg-config --cflags --libs x11')
env.ParseConfig('pkg-config --cflags --libs libcrypto')
env.ParseConfig('pkg-config --cflags --libs nss')
env.ParseConfig('pkg-config --cflags --libs openh264')
env.ParseConfig('pkg-config --cflags --libs jsoncpp')
env.ParseConfig('pkg-config --cflags --libs sigc++-2.0')

# Set sources for H264WebRTC
sources = [	'main.cpp',
            'json_parser/json_parser.cpp',
            'peer_manager_imp.cpp',
            'observers/set_sdp_observer.cpp',
            'observers/peer_connection_observer.cpp',
            'observers/create_sdp_observer.cpp',
            'video_encoder/h264webrtc_video_encoder_factory.cpp',
            'video_encoder/h264/open_h264_video_encoder_impl.cpp',
		  ]

# Build
env.Program('#bin/H264WebRTC', sources)