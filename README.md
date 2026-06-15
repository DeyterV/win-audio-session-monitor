# win-audio-session-monitor

OBS Studio plugin that adds a dock panel listing all active Windows audio sessions with their process name, PID, state, and volume level.

## Features

- Shows all audio sessions from the default playback device
- Columns: executable name, PID, state (Active / Inactive / Expired), volume %
- Auto-refresh every 3 seconds (can be toggled off)
- Manual refresh button

## Requirements

- Windows 10 or later
- OBS Studio 31.0 or later

## Installation

1. Download the latest `.zip` from [Releases](https://github.com/DeyterV/win-audio-session-monitor/releases)
2. Extract the zip — it produces a `win-audio-session-monitor/` folder
3. Place that folder into one of:
   - `%APPDATA%\obs-studio\plugins\` — per-user install, no admin rights required
   - `<obs-dir>\obs-plugins\64bit\` for the `.dll` and `<obs-dir>\data\obs-plugins\win-audio-session-monitor\` for the `data\` folder — system-wide install
4. Restart OBS Studio
5. Open the dock via **View → Docks → Audio Session Monitor**

## Building from source

Requires Visual Studio 2022 and CMake 3.28+.

```
git clone https://github.com/DeyterV/win-audio-session-monitor.git
cd win-audio-session-monitor
cmake --preset windows-x64
cmake --build --preset windows-x64
```

Dependencies (OBS headers, Qt6) are downloaded automatically by the build system.

## License

[GPL-2.0-or-later](LICENSE)
