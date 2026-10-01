# Lightswitch

Qt 6 / C++17 light switch UI for a 720x720 display. Design spec: `docs/superpowers/specs/`, plan: `docs/superpowers/plans/`.

## Build system
- **Windows main build: `Lightswitch.slnx`** (Visual Studio 2026). Projects in `vs/` (`LightswitchCore` static lib, `Lightswitch` app, one project per test), shared settings in `msbuild/` (`Common.props` configurations/output/Qt paths, `Qt.targets` moc+rcc, `Test.targets`). Qt location: `Lightswitch.user.props` (not committed, see `.example`) or `QTDIR`.
- **Linux: CMake** (`CMakeLists.txt`). It reads source lists, the qrc and the test list from the slnx/vcxproj files (`cmake/VisualStudioProject.cmake`), so the slnx stays the single source of truth. New source file or test: add it to the vcxproj / slnx, not to CMake.
- Commands: Linux `./BuildAndRun.sh [debug|release|debug_level_log|release_level_log] [--clean] [--no-test] [--no-run] [-j N] [-- app args]`; Windows `msbuild Lightswitch.slnx /p:Configuration=Debug /p:Platform=x64 /m`, tests `./RunTests.ps1 -Configuration Debug`.
- Output always in `bin/<system><arch>/<config>/` with config directories `debug`, `release`, `debug_level_log`, `release_level_log`. Log level is the compile definition `LIGHTSWITCH_LOG_LEVEL`, separate from optimization settings.
- Application arguments: `--fullscreen`, `--config <file>` (default `lightswitch.ini` next to the executable).

## Layout
- `src/core` clock, light, alarm; `src/weather` Open-Meteo client and dot model; `src/calendar` calendar providers; `src/app` configuration, logging, `AppController`, `main.cpp`.
- `qml/` Qt Quick UI, bundled by `qml/Lightswitch.qrc` (prefix `/qt/qml/Lightswitch/Ui`, `qmldir` declares the `Theme` singleton); `resources/fonts` Manufaktur fonts; `config/lightswitch.ini` default configuration; `tests/` QtTest sources.

## Conventions
Global C++ standard applies (PascalCase methods/classes/files, `m_` members, camelCase variables, English ASCII comments, descriptions above `.cpp` methods and `.h` classes). New class with `Q_OBJECT`: add its header as `QtMoc` in the vcxproj. Test with `#include "X.moc"`: add the `.cpp` as both `ClCompile` and `QtMocInclude`.
