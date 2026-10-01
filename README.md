# Lightswitch

Light switch UI for a 720x720 display, built with C++17 and Qt 6 (Qt Quick). Tiles: Time, Light, Weather (rain forecast of the next 24 hours as a 12x5 dot matrix), Calendar (Google Calendar, or dummy events without credentials) and Alarm (switches the light on at a set time).

## Prerequisites

- Qt 6.4 or newer (Core, Network, Gui, Qml, Quick)
- Windows: Visual Studio 2026 (MSVC toolchain)
- Linux: CMake 3.22 or newer and a C++17 compiler; on apt based systems `./BuildAndRun.sh --install-deps` installs everything

## Windows: Visual Studio 2026 (main build)

`Lightswitch.slnx` is the main build. The projects live in `vs/`, shared settings in `msbuild/`.

1. Tell the projects where Qt is: copy `Lightswitch.user.props.example` to `Lightswitch.user.props` and set `QtRoot` (for example `C:\Qt\6.8.0\msvc2022_64`), or set the `QTDIR` environment variable.
2. Open `Lightswitch.slnx`, pick a configuration (`Debug`, `Release`, `DebugLevelLog`, `ReleaseLevelLog`, platform `x64`) and build. `Lightswitch` is the startup project.
3. Run the tests with `./RunTests.ps1 -Configuration Debug` (every test is its own project under the `Tests` folder of the solution).

Command line: `msbuild Lightswitch.slnx /p:Configuration=Release /p:Platform=x64 /m`.

Output lands in `bin/windowsx64/<configuration>/` (`debug`, `release`, `debug_level_log`, `release_level_log`).

## Linux: CMake

CMake reads its source lists and the test list from `Lightswitch.slnx` and the projects in `vs/`, so both builds always compile the same files. To add a source file or test, add it to the Visual Studio project; CMake follows automatically.

```bash
./BuildAndRun.sh                          # debug build, tests, start
./BuildAndRun.sh release --clean --no-run -j 8
./BuildAndRun.sh debug_level_log -- --fullscreen
```

Output lands in `bin/linux<arch>/<configuration>/`.

## Application options

- `--fullscreen` fills the screen (the 720x720 design is scaled to fit).
- `--config <file>` uses another configuration file (default: `lightswitch.ini` next to the executable).

## Configuration (`config/lightswitch.ini`)

```ini
[location]
name=Graz             ; shown in the settings, presets can be selected on the device
latitude=47.0707     ; weather location in degrees
longitude=15.4395

[alarm]
enabled=true         ; global alarm switch
time=06:45           ; HH:mm, default time of all days
mon=06:30            ; optional time per weekday (mon..sun)
days=Mon,Tue,Wed,Thu,Fri   ; days on which the alarm rings

[nightmode]
enabled=true         ; screen goes dark after 5 minutes without input ...
start=22:00          ; ... between start and end (may span midnight),
end=06:00            ; but never while the light is on
```

Night mode shows only a dark clock on a black screen; a touch wakes it up.

## Operation

- Tap the Time tile: night mode starts immediately (any touch wakes it up again).
- Long press the Alarm tile: alarm settings with the global switch and an on/off flag and time for each weekday. The tile shows the time of the next alarm (OFF when none).
- Long press the Time tile: global settings for the weather location (preset cities) and night mode (on/off, start and end time). Changes are saved to the configuration file.

## Google Calendar

The Calendar tile shows the next event within the next 7 days (all-day events show only the date, long titles use up to three lines). Real events take precedence: reminders and birthdays are only shown there when no event is coming up. Tapping the tile opens the list of all upcoming events, reminders (Google Tasks) and birthdays (Google contacts). Without Google credentials it shows random dummy entries.

1. In the [Google Cloud Console](https://console.cloud.google.com/) enable the Google Calendar API and the Google Tasks API (reminders; Google only provides their due date, not the time) and create an OAuth client of type **Desktop app** (add your account as test user while the consent screen is in testing mode).
2. Copy `config/google.ini.example` to `google.ini` next to `lightswitch.ini` and enter the client id and secret. The file is git-ignored.
3. Start the app and tap the Calendar tile ("Link Google"). The Google sign-in opens in the default browser **on the same machine** (the address is also written to the log); after granting read access to calendar and tasks the refresh token is stored in `google-token.ini` next to the configuration file.

Google does not allow the calendar scope for the device code flow, so the one-time sign-in needs a browser on the machine running the app. For a headless device, link it once on a PC and copy `google-token.ini` (and `google.ini`) next to the device's `lightswitch.ini`.

Missing or invalid values fall back to these defaults. Weather data comes from [Open-Meteo](https://open-meteo.com/) (no API key needed).
