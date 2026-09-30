# Lightswitch

Light switch UI for a 720x720 display, built with C++17 and Qt 6 (Qt Quick). Tiles: Time, Light, Weather (rain forecast of the next 24 hours as a 12x5 dot matrix), Calendar (dummy events for now) and Alarm (switches the light on at a set time).

## Prerequisites

- Qt 6.4 or newer (Core, Network, Gui, Qml, Quick)
- CMake 3.22 or newer
- Windows: Visual Studio 2026 (MSVC toolchain)
- Linux: a C++17 compiler; on apt based systems `./BuildAndRun.sh --install-deps` installs everything

## Build and run

Linux:

```bash
./BuildAndRun.sh                          # debug build, tests, start
./BuildAndRun.sh release --clean --no-run -j 8
./BuildAndRun.sh debug_level_log -- --fullscreen
```

Windows (Developer PowerShell for VS 2026):

```powershell
$env:CMAKE_PREFIX_PATH = "C:/Qt/6.8.0/msvc2022_64"   # adjust to your Qt install
cmake --preset windows-x64
cmake --build --preset windows-x64-debug
ctest --test-dir bin/windowsx64/vs -C Debug --output-on-failure
bin/windowsx64/debug/Lightswitch.exe
```

Visual Studio 2026 IDE: `./GenerateSolution.ps1 -QtPrefix C:/Qt/6.8.0/msvc2022_64 -Open` configures the project and opens `bin/windowsx64/vs/Lightswitch.slnx`. The solution is generated on purpose and not committed, because CMake writes absolute paths of your machine into it.

Configurations: `debug`, `release`, `debug_level_log`, `release_level_log`. All output lands in `bin/<system><arch>/<configuration>/`.

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
