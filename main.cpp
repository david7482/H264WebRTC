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
gchar *session_id = NULL, *peer_id = NULL;
guint32 client_id = 0;
int mode = 0;
GMainLoop *main_loop;
PeerManager *peer_manager = NULL;
bool is_peer_joined = false;

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

static void answer_finish(SoupSession *soup_session, GAsyncResult *result, gpointer user_data)
{
    GInputStream *input_stream;
    input_stream = soup_session_send_finish(soup_session, result, NULL);
    if (!input_stream)
        g_warning("Failed to send back to server");
    else
        g_object_unref(input_stream);
}

void signal_sdp_feedback(Json::Value value)
{
    Json::Value jmessage;
    jmessage["sdp"] = value;

    std::string sdp = JsonValueToString(jmessage);
    g_debug("%s", sdp.c_str());

    gchar *url = g_strdup_printf("%s/ctos/%s/%u/%s", h264webrtc::server_address, session_id, client_id, peer_id);
    SoupMessage *soup_message = soup_message_new("POST", url);
    soup_message_set_request(soup_message, "application/json", SOUP_MEMORY_COPY, sdp.c_str(), sdp.length());
    g_free(url);

    SoupSession *soup_session = soup_session_new();
    soup_session_send_async(soup_session, soup_message, NULL, (GAsyncReadyCallback) answer_finish, NULL);
}

void signal_candidate_feedback(Json::Value value)
{
    Json::Value jmessage;
    jmessage["candidate"] = value;

    std::string candidate = JsonValueToString(jmessage);
    g_debug("%s", candidate.c_str());

    gchar *url = g_strdup_printf("%s/ctos/%s/%u/%s", h264webrtc::server_address, session_id, client_id, peer_id);
    SoupSession *soup_session = soup_session_new();
    SoupMessage *soup_message = soup_message_new("POST", url);
    g_free(url);
    soup_message_set_request(soup_message, "application/json", SOUP_MEMORY_COPY, candidate.c_str(), candidate.length());
    soup_session_send_async(soup_session, soup_message, NULL, (GAsyncReadyCallback) answer_finish, NULL);
}

void eventstream_line_read(GDataInputStream *input_stream, GAsyncResult *result, gpointer user_data)
{
    gchar *line;
    gsize line_length;

    line = g_data_input_stream_read_line_finish_utf8(input_stream, result, &line_length, NULL);
    g_return_if_fail(line);

    //g_debug("%s", line);
    if (!is_peer_joined && g_strstr_len(line, MIN(line_length, 10), "event:join")) {

        is_peer_joined = true;
        g_debug("Peer joined");

    } else if (is_peer_joined && g_strstr_len(line, MIN(line_length, 11), "event:leave")) {

        peer_manager->deletePeerConnection(peer_id);
        g_free(peer_id);
        peer_id = NULL;
        is_peer_joined = false;
        g_debug("Peer leave");

    } else if (is_peer_joined && g_strstr_len(line, MIN(line_length, 5), "data:")) {

        if (g_strstr_len(line, MIN(line_length, 6), "data:{")) {
            // Handle the "data:{"sdp":...}" and "data:{"candidate":...}" case
            Json::Reader reader;
            Json::Value jmessage;
            if (reader.parse(line + 5, jmessage)) {
                Json::Value value;
                if (GetValueFromJsonObject(jmessage, "sdp", &value)) {

                    g_debug("Get sdp ->");
                    peer_manager->setOffser(peer_id, value);
                    g_debug("Get sdp <-");

                } else if (GetValueFromJsonObject(jmessage, "candidate", &value)) {

                    g_debug("Get candidate ->");
                    peer_manager->addIceCandidate(peer_id, value);
                    g_debug("Get candidate <-");

                } else {
                    g_debug("Invalid json message: %s", line + 5);
                }
            }
        } else {
            // Handle the peer id
            peer_id = g_strndup(line + 5, line_length - 5);
            g_debug("Get peer_id: %s", peer_id);
        }

    }

    g_free(line);

    read_eventstream_line(input_stream, NULL);
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

    if (h264webrtc::is_verbose_mode) {
        rtc::LogMessage::LogToDebug((h264webrtc::is_verbose_mode) ? rtc::INFO : rtc::LERROR);
        rtc::LogMessage::LogTimestamps();
        rtc::LogMessage::LogThreads();
        g_setenv("G_MESSAGES_DEBUG", "all", TRUE);
    }
    rtc::InitializeSSL();

    // Create the soup session and connect to signal server
    h264webrtc::client_id = g_random_int();
    gchar *url = g_strdup_printf("%s/stoc/%s/%u", h264webrtc::server_address, h264webrtc::session_id, h264webrtc::client_id);
    if (url) {
        h264webrtc::send_eventsource_request(url);
        g_free(url);
    }

    // webrtc server
    h264webrtc::peer_manager = h264webrtc::PeerManager::Create("stun.l.google.com:19302");
    h264webrtc::peer_manager->signal_sdp_feedback.connect(sigc::ptr_fun(h264webrtc::signal_sdp_feedback));
    h264webrtc::peer_manager->signal_candidate_feedback.connect(sigc::ptr_fun(h264webrtc::signal_candidate_feedback));

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