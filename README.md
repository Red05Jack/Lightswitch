# Lightswitch

Light switch UI for a 720x720 display, built with C++17 and Qt 6 (Qt Quick). Tiles: Time, Light, Weather (rain forecast of the next 24 hours as a 12x5 dot matrix), Calendar (dummy events for now) and Alarm (switches the light on at a set time).

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
latitude=48.1374     ; weather location in degrees
longitude=11.5755

[alarm]
time=06:45           ; HH:mm, switches the light on
days=Mon,Tue,Wed,Thu,Fri
```

Missing or invalid values fall back to these defaults. Weather data comes from [Open-Meteo](https://open-meteo.com/) (no API key needed).
