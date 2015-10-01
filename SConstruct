import os

debug = ARGUMENTS.get('debug', 0)
webrtc_root_path = ARGUMENTS.get('webrtc-path', '')

# Set our required libraries
libraries 		= []
library_paths 	= ''
cppDefines 		= {}
cFlags 		    = ['-Wall']

# For debug build
if int(debug):
	cFlags += ['-g', '-O0']

# Define the attributes of the build environment shared
env = Environment(CPPPATH = ['.'])
env.Append(LIBS 		= libraries)
env.Append(LIBPATH 		= library_paths)
env.Append(CPPDEFINES 	= cppDefines)
env.Append(CFLAGS 		= cFlags)
env.Append(CPPFLAGS 	= cFlags)

WEBRTC_PKG_ROOT_PATH = '/opt/webrtc'
# For debug build
if int(debug):
    env['ENV']['PKG_CONFIG_PATH'] = '%s/lib/Debug/pkgconfig' % WEBRTC_PKG_ROOT_PATH
else:
    env['ENV']['PKG_CONFIG_PATH'] = '%s/lib/Release/pkgconfig' % WEBRTC_PKG_ROOT_PATH

env.ParseConfig('pkg-config --cflags --libs glib-2.0')
env.ParseConfig('pkg-config --cflags --libs libsoup-2.4')
env.ParseConfig('pkg-config --cflags --libs --define-variable=prefix=%s libwebrtc_full' % WEBRTC_PKG_ROOT_PATH)
env.ParseConfig('pkg-config --cflags --libs x11')
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
            'codecs/codec_factory.cpp',
            'codecs/h264/open_h264_encoder_impl.cpp',
            'codecs/h264/open_h264_decoder_impl.cpp',
		  ]

# Build
env.Program('#bin/H264WebRTC', sources)