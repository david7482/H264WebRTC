#include "peer_manager.h"
#include "json_parser/json_parser.h"

#include "glib.h"
#include "glib-unix.h"
#include "libsoup/soup.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/thread.h"

namespace h264webrtc
{

void read_eventstream_line(GDataInputStream *input_stream, gpointer user_data);

const gchar *server_address = "http://demo.openwebrtc.io:38080";
bool is_verbose_mode = false;
gchar *session_id = NULL;
int mode = 0;
GMainLoop *main_loop;
PeerManager *peer_manager = NULL;

GOptionEntry opt_entries[] = {
        {"verbose",   'v', 0, G_OPTION_ARG_NONE,   &is_verbose_mode, "Enable verbose mode (default: disable)",                                             NULL},
        {"mode",      'm', 0, G_OPTION_ARG_INT,    &mode,            "Set camera mode (default: 0) (0: YuvFrameGenerator, 1: QIC camera, 2: V4L2 device)", NULL},
        {"sessionid", 's', 0, G_OPTION_ARG_STRING, &session_id,      "Specify the session id",                                                             NULL},
        {NULL},
};

gboolean on_sig_handler(gpointer userdata)
{
    g_debug("Quit");
    g_main_loop_quit(main_loop);
    return G_SOURCE_REMOVE;
}

/*void process_jumbo_message(const std::string jumbo_message)
{
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(jumbo_message, jmessage)) {
        g_critical("Fail to parse jumbo message");
        return;
    }

    std::string message_type;
    g_return_if_fail(rtc::GetStringFromJsonObject(jmessage, "message_type", &message_type));
    {
        if (message_type == "response") {
            std::string response_type;
            g_return_if_fail(rtc::GetStringFromJsonObject(jmessage, "response_type", &response_type));
            g_debug("response_type: %s", response_type.c_str());
        }
        else if (message_type == "request") {
            std::string request_type;
            g_return_if_fail(rtc::GetStringFromJsonObject(jmessage, "request_type", &request_type));
            {
                g_debug("request_type: %s", request_type.c_str());
                if (request_type == "sdp") {
                    std::string usersession_id;
                    g_return_if_fail(rtc::GetStringFromJsonObject(jmessage, "usersession_id", &usersession_id));

                    Json::Value jpayload;
                    g_return_if_fail(rtc::GetValueFromJsonObject(jmessage, "payload", &jpayload));

                    peer_manager->setOffser(usersession_id, rtc::JsonValueToString(jpayload));
                }
                else if (request_type == "hangup") {
                    std::string usersession_id;
                    g_return_if_fail(rtc::GetStringFromJsonObject(jmessage, "usersession_id", &usersession_id));

                    peer_manager->deletePeerConnection(usersession_id);
                }
            }
        }
        else {
            g_debug("Fail to parse jumbo message");
        }
    }
}*/

/*void send_jumbo_message(const std::string &type, const Json::Value &jumbo_message)
{
Json::Value jmessage;
jmessage["message_type"] = "response";
jmessage["response_type"] = type;
jmessage["payload"] = jumbo_message;

std::string init = rtc::JsonValueToString(jmessage);
g_debug("msg: %s", init.c_str());
soup_websocket_connection_send_text(conn, init.c_str());
}*/

void eventstream_line_read(GDataInputStream *input_stream, GAsyncResult *result, gpointer user_data)
{
    gchar *line;
    gsize line_length;
    gboolean peer_joined = GPOINTER_TO_UINT(user_data);

    line = g_data_input_stream_read_line_finish_utf8(input_stream, result, &line_length, NULL);
    g_return_if_fail(line);

    //g_debug("%s", line);
    if (g_strstr_len(line, MIN(line_length, 6), "data:{")) {
        Json::Reader reader;
        Json::Value jmessage;
        if (reader.parse(line + 5, jmessage)) {
            Json::Value value;
            if (GetValueFromJsonObject(jmessage, "sdp", &value)) {
                g_debug("Get sdp");
                peer_manager->setOffser("lalala", value);
            } else if (GetValueFromJsonObject(jmessage, "candidate", &value)) {
                g_debug("Get candidate");
                peer_manager->addIceCandidate("lalala", value);
            } else {
                g_critical("Invalid json message");
            }
        }
    }

    g_free(line);

    read_eventstream_line(input_stream, GUINT_TO_POINTER(peer_joined));
}

void read_eventstream_line(GDataInputStream *input_stream, gpointer user_data)
{
    g_data_input_stream_read_line_async(
            input_stream,
            G_PRIORITY_DEFAULT,
            NULL,
            (GAsyncReadyCallback) eventstream_line_read,
            user_data
    );
}

void eventsource_request_sent(SoupSession *soup_session, GAsyncResult *result, gpointer user_data)
{
    GInputStream *input_stream;
    GDataInputStream *data_input_stream;

    input_stream = soup_session_send_finish(soup_session, result, NULL);
    if (input_stream) {
        data_input_stream = g_data_input_stream_new(input_stream);
        read_eventstream_line(data_input_stream, user_data);
    } else {
        g_warning("Failed to connect to the server");
    }
}

void send_eventsource_request(const gchar *url)
{
    SoupSession *soup_session;
    SoupMessage *soup_message;

    soup_session = soup_session_new();
    soup_message = soup_message_new("GET", url);
    soup_session_send_async(
            soup_session,
            soup_message,
            NULL,
            (GAsyncReadyCallback) eventsource_request_sent,
            NULL
    );
}

} // namespace h264webrtc

int main(int argc, char **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    context = g_option_context_new("- H264Webrtc Server");
    g_option_context_add_main_entries(context, h264webrtc::opt_entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_error_free(error);
        return 1;
    }

    if (h264webrtc::session_id == NULL) {
        g_print("%s", g_option_context_get_help(context, true, NULL));
        return 1;
    }

    rtc::LogMessage::LogToDebug((h264webrtc::is_verbose_mode) ? rtc::INFO : rtc::LERROR);
    rtc::LogMessage::LogTimestamps();
    rtc::LogMessage::LogThreads();
    rtc::InitializeSSL();

    // Create the soup session and connect to signal server
    gchar *url = g_strdup_printf("%s/stoc/%s/%u", h264webrtc::server_address, h264webrtc::session_id, g_random_int());
    if (url) {
        h264webrtc::send_eventsource_request(url);
        g_free(url);
    }

    // webrtc server
    h264webrtc::peer_manager = h264webrtc::PeerManager::Create("stun.l.google.com:19302");

    // Create and start the main loop
    h264webrtc::main_loop = g_main_loop_new(NULL, FALSE);
    g_unix_signal_add(SIGINT, (GSourceFunc) h264webrtc::on_sig_handler, NULL);
    g_main_loop_run(h264webrtc::main_loop);

    g_main_loop_unref(h264webrtc::main_loop);

    // Clean up
    delete h264webrtc::peer_manager;

    rtc::CleanupSSL();

    return 0;
}