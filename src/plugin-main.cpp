#include <obs-module.h>
#include <obs-frontend-api.h>

#include "audio-session-dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("win-audio-session-monitor", "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Audio session monitor dock panel";
}

static void frontend_event(enum obs_frontend_event event, void *)
{
	if (event != OBS_FRONTEND_EVENT_FINISHED_LOADING)
		return;

	auto *dock = new AudioSessionDock();
	obs_frontend_add_dock_by_id("win-audio-session-monitor",
				    obs_module_text("AudioSessionMonitor"), dock);

	obs_frontend_remove_event_callback(frontend_event, nullptr);
}

bool obs_module_load(void)
{
	obs_frontend_add_event_callback(frontend_event, nullptr);
	return true;
}

void obs_module_unload(void) {}
