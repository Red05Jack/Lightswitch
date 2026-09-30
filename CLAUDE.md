# Lightswitch

Qt 6 / C++17 light switch UI for a 720x720 display. Design spec: `docs/superpowers/specs/`, plan: `docs/superpowers/plans/`.

## Commands
- Linux: `./BuildAndRun.sh [debug|release|debug_level_log|release_level_log] [--clean] [--no-test] [--no-run] [-j N] [-- app args]`
- Windows (Developer PowerShell for VS 2026, Qt via `$env:CMAKE_PREFIX_PATH`, e.g. `C:/Qt/6.8.0/msvc2022_64`): `cmake --preset windows-x64`, `cmake --build --preset windows-x64-debug`, `ctest --test-dir bin/windowsx64/vs -C Debug --output-on-failure`. Visual Studio solution (`bin/windowsx64/vs/Lightswitch.slnx`, generated, not committed): `./GenerateSolution.ps1 -QtPrefix <Qt kit> [-Open]`.
- Application arguments: `--fullscreen`, `--config <file>` (default `lightswitch.ini` next to the executable).

## Layout
- `src/core` clock, light, alarm; `src/weather` Open-Meteo client and dot model; `src/calendar` calendar providers; `src/app` configuration, logging, `AppController`, `main.cpp`.
- `qml/` Qt Quick UI module `Lightswitch.Ui`; `resources/fonts` Manufaktur fonts; `config/lightswitch.ini` default configuration; `tests/` QtTest + CTest.
- Output always in `bin/<system><arch>/<config>/`. Log level is the compile definition `LIGHTSWITCH_LOG_LEVEL`, separate from the build configuration.

## Conventions
Global C++ standard applies (PascalCase methods/classes/files, `m_` members, camelCase variables, English ASCII comments, descriptions above `.cpp` methods and `.h` classes). New logic class: add its `.cpp` to `LightswitchCore` in `CMakeLists.txt` and a test via `add_lightswitch_test` in `tests/CMakeLists.txt`.
