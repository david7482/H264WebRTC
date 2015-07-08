#include "glib.h"
#include "set_sdp_observer.h"

SetSDPObserver *SetSDPObserver::Create()
{
    return new rtc::RefCountedObject<SetSDPObserver>();
}

void SetSDPObserver::OnSuccess()
{
    g_debug("Success to set SDP");
}

void SetSDPObserver::OnFailure(const std::string &error)
{
    g_debug("Fail to set SDP: %s", error.c_str());
}