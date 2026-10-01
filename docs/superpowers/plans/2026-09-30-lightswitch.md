# Lightswitch Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A 720x720 light switch UI (Time, Light, Weather, Calendar, Alarm tiles) in C++17 and Qt Quick that builds on Windows (VS 2026) and Linux (CMake).

**Architecture:** All logic lives in a static C++ library (`LightswitchCore`) made of small QObject models and services behind injectable interfaces (`IClock`, `ILightController`, `ICalendarProvider`). A thin Qt Quick module (`LightswitchUi`) renders the tiles from the SVG design and reads the models through one `AppController` context property. Every logic class has a QtTest registered with CTest.

**Tech Stack:** C++17, Qt 6.4+ (Core, Network, Gui, Qml, Quick, Test), CMake 3.22+, Open-Meteo HTTP API, bash (`BuildAndRun.sh`).

**Spec:** `docs/superpowers/specs/2026-09-30-lightswitch-design.md`

## Global Constraints

- Language: C++17, no compiler extensions. All identifiers, comments and file names in English, ASCII only. Non-ASCII characters in strings are written as escapes (`QChar(0x2022)` bullet, `QChar(0x00B0)` degree sign).
- Naming: classes and file names PascalCase without underscores; methods and functions PascalCase; variables camelCase; booleans start with `is`/`has`/`can`/`should`; member variables `m_camelCase`; raw pointer variables prefixed with `p`. Indentation with tabs.
- One class per `.h`/`.cpp` pair. Class description comment only in the `.h` directly above the class. Method/function description comment only in the `.cpp` directly above the implementation (also for free functions in anonymous namespaces). No description comments on methods in headers.
- No raw `new`/`delete`; smart pointers or members by value. Raw pointers only for non-owning Qt object access.
- Metric units and degrees Celsius only.
- Output layout: every binary and build artifact under `<root>/bin/<system><arch>/<config>/` with config directories `debug`, `release`, `debug_level_log`, `release_level_log` (Windows IDE/multi-config generator keeps its project files in `bin/windowsx64/vs/`). No `build/`, `out/` or `cmake-build-*` directories.
- Configurations: `Debug`, `Release`, `DebugLevelLog`, `ReleaseLevelLog`. The log level is a separate compile definition `LIGHTSWITCH_LOG_LEVEL` (0 = no debug output, 2 = default, 3 = verbose). `ReleaseLevelLog` keeps the Release optimization flags.
- Every source file ends with a trailing newline.
- Display: fixed 720x720 design, fonts Manufaktur Bold/Black, colors exactly as in the spec.
- Commits use conventional prefixes (`feat:`, `fix:`, `chore:`, `docs:`, `test:`) and end with the trailer line `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`.

## Review Focus

Inputs and conditions the spec implies but does not spell out, most likely to bite first:

- Open-Meteo returns `null` precipitation values, fewer than 24 remaining hours, or a current hour that is missing from the hourly list: no crash, sensible zeros, `nullopt` when unusable (Task 5).
- Precipitation values outside 0..100 or a short hourly list: clamped, missing columns show 0 dots (Task 6).
- The app starts offline: the weather tile shows `--` and stays stable (Task 6); a later successful fetch replaces it.
- `lightswitch.ini` is missing, empty or contains garbage (`latitude=abc`, `time=25:99`, unknown day names): defaults are used and nothing crashes (Task 4).
- The alarm fires at most once per day, fires again the next active day, never fires on inactive days, and never switches the light off (Task 3).
- Calendar provider returns no events: the tile shows `No events` (Task 8).

---

## Build and test commands used by all tasks

Windows (run in **Developer PowerShell for VS 2026** so that `cmake` is on PATH; Qt location via environment variable, adjust the version):

```powershell
$env:CMAKE_PREFIX_PATH = "C:/Qt/6.8.0/msvc2022_64"
cmake --preset windows-x64
cmake --build --preset windows-x64-debug
ctest --test-dir bin/windowsx64/vs -C Debug --output-on-failure
```

Build one target only: `cmake --build --preset windows-x64-debug --target <TestName>`.
Run one test: `ctest --test-dir bin/windowsx64/vs -C Debug -R <TestName> --output-on-failure`.

Linux:

```bash
./BuildAndRun.sh debug --no-run
```

Run one test on Linux: `ctest --test-dir bin/linuxx64/debug -R <TestName> --output-on-failure`.

Where a step says "run the build", use the build command of the current platform; "run the tests" means the ctest command.

---

### Task 1: Toolchain, build system foundation and clock model

**Files:**
- Create: `cmake/BuildLayout.cmake`, `CMakeLists.txt`, `CMakePresets.json`, `BuildAndRun.sh`, `CLAUDE.md`
- Create: `resources/fonts/Manufaktur-Bold.ttf`, `resources/fonts/Manufaktur-Black.ttf` (copied from `C:\Users\User\AppData\Local\Temp\`)
- Create: `src/core/IClock.h`, `src/core/SystemClock.h`, `src/core/SystemClock.cpp`, `src/core/ClockModel.h`, `src/core/ClockModel.cpp`
- Create: `tests/CMakeLists.txt`, `tests/FakeClock.h`, `tests/ClockModelTest.cpp`
- Modify: `.gitignore`

**Interfaces:**
- Produces: `class IClock { virtual QDateTime Now() const = 0; }`; `class SystemClock : IClock`; `class ClockModel : QObject` with `explicit ClockModel(const IClock& clock, QObject* pParent = nullptr)`, `const QString& TimeText() const`, `const QString& DateText() const`, `void Start()`, `void Refresh()`, signal `Changed()`, properties `timeText`, `dateText`. Test helper `FakeClock(const QDateTime&)` with `SetNow(const QDateTime&)`. CMake helper `add_lightswitch_test(name)` in `tests/CMakeLists.txt`; target `LightswitchCore`; variable `LIGHTSWITCH_QT_BIN_DIR`.

- [ ] **Step 1: Verify and install prerequisites**

Neither `cmake` nor Qt is installed on this machine (verified 2026-09-30); Visual Studio 2026 is installed at `C:\Program Files\Microsoft Visual Studio\18\Insiders`. Ask the user before downloading anything. Needed:

- Qt 6.4 or newer, MSVC 2022 64-bit kit, with modules Qt Quick, Qt Network and Qt Shader Tools. Via Qt Online Installer, or `pip install aqtinstall` then `aqt install-qt windows desktop 6.8.0 win64_msvc2022_64 -O C:/Qt`.
- CMake 3.22+ with the `Visual Studio 18 2026` generator (needs CMake 4.2 or newer). The CMake bundled with VS 2026 is on PATH inside Developer PowerShell for VS 2026.

Verify:

```powershell
cmake --version
cmake --help | Select-String "Visual Studio 18 2026"
Test-Path "C:/Qt/6.8.0/msvc2022_64/bin/windeployqt.exe"
```

Expected: a CMake version, a line listing `Visual Studio 18 2026`, and `True`. If the generator is missing, run `cmake --help | Select-String "Visual Studio"` and use the newest listed generator name in `CMakePresets.json` instead (and in Step 4 below).

- [ ] **Step 2: Copy the fonts and extend `.gitignore`**

```bash
mkdir -p resources/fonts
cp /c/Users/User/AppData/Local/Temp/Manufaktur-Bold.ttf /c/Users/User/AppData/Local/Temp/Manufaktur-Black.ttf resources/fonts/
```

Append to `.gitignore` (read it first, do not duplicate entries):

```
bin/
CMakeUserPresets.json
.vs/
*.user
```

- [ ] **Step 3: Write `cmake/BuildLayout.cmake`**

```cmake
# Defines the standard configurations and the output layout
# <root>/bin/<system><architecture>/<configuration>/.
# Must be included after project().

set(LIGHTSWITCH_CONFIGURATIONS Debug Release DebugLevelLog ReleaseLevelLog)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" _system)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _processor)
if(_processor MATCHES "^(amd64|x86_64|x64)$")
	set(_architecture x64)
elseif(_processor MATCHES "^(arm64|aarch64)$")
	set(_architecture arm64)
elseif(_processor MATCHES "^(x86|i[3-6]86)$")
	set(_architecture x86)
else()
	message(FATAL_ERROR "Unsupported processor: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
	set(CMAKE_CONFIGURATION_TYPES "${LIGHTSWITCH_CONFIGURATIONS}" CACHE STRING "" FORCE)
else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
	endif()
	if(NOT CMAKE_BUILD_TYPE IN_LIST LIGHTSWITCH_CONFIGURATIONS)
		message(FATAL_ERROR "CMAKE_BUILD_TYPE must be one of: ${LIGHTSWITCH_CONFIGURATIONS}")
	endif()
endif()

# The level-log configurations reuse the flags of their base configuration.
foreach(_language C CXX)
	set(CMAKE_${_language}_FLAGS_DEBUGLEVELLOG "${CMAKE_${_language}_FLAGS_DEBUG}" CACHE STRING "" FORCE)
	set(CMAKE_${_language}_FLAGS_RELEASELEVELLOG "${CMAKE_${_language}_FLAGS_RELEASE}" CACHE STRING "" FORCE)
endforeach()
foreach(_linkerKind EXE SHARED MODULE STATIC)
	set(CMAKE_${_linkerKind}_LINKER_FLAGS_DEBUGLEVELLOG "${CMAKE_${_linkerKind}_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
	set(CMAKE_${_linkerKind}_LINKER_FLAGS_RELEASELEVELLOG "${CMAKE_${_linkerKind}_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
endforeach()

# Qt only ships Debug and Release libraries.
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUGLEVELLOG Debug Release)
set(CMAKE_MAP_IMPORTED_CONFIG_RELEASELEVELLOG Release)

# MSVC: debug-like configurations must use the debug runtime to match the Qt debug libraries.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug,DebugLevelLog>:Debug>DLL")

foreach(_config IN LISTS LIGHTSWITCH_CONFIGURATIONS)
	string(TOUPPER "${_config}" _upperConfig)
	string(REGEX REPLACE "([a-z])([A-Z])" "\\1_\\2" _directoryName "${_config}")
	string(TOLOWER "${_directoryName}" _directoryName)
	set(_outputDirectory "${CMAKE_SOURCE_DIR}/bin/${_system}${_architecture}/${_directoryName}")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_upperConfig} "${_outputDirectory}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_upperConfig} "${_outputDirectory}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_upperConfig} "${_outputDirectory}")
endforeach()

set(LIGHTSWITCH_LOG_LEVEL_DEFINITION
	$<$<CONFIG:Debug>:LIGHTSWITCH_LOG_LEVEL=2>
	$<$<CONFIG:Release>:LIGHTSWITCH_LOG_LEVEL=0>
	$<$<CONFIG:DebugLevelLog,ReleaseLevelLog>:LIGHTSWITCH_LOG_LEVEL=3>
)
```

- [ ] **Step 4: Write `CMakePresets.json`**

```json
{
	"version": 3,
	"cmakeMinimumRequired": { "major": 3, "minor": 22, "patch": 0 },
	"configurePresets": [
		{
			"name": "linux-base",
			"hidden": true,
			"generator": "Unix Makefiles",
			"condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Linux" }
		},
		{ "name": "linux-debug", "inherits": "linux-base", "binaryDir": "${sourceDir}/bin/linuxx64/debug", "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" } },
		{ "name": "linux-release", "inherits": "linux-base", "binaryDir": "${sourceDir}/bin/linuxx64/release", "cacheVariables": { "CMAKE_BUILD_TYPE": "Release" } },
		{ "name": "linux-debug-level-log", "inherits": "linux-base", "binaryDir": "${sourceDir}/bin/linuxx64/debug_level_log", "cacheVariables": { "CMAKE_BUILD_TYPE": "DebugLevelLog" } },
		{ "name": "linux-release-level-log", "inherits": "linux-base", "binaryDir": "${sourceDir}/bin/linuxx64/release_level_log", "cacheVariables": { "CMAKE_BUILD_TYPE": "ReleaseLevelLog" } },
		{ "name": "linux-arm64-debug", "inherits": "linux-base", "binaryDir": "${sourceDir}/bin/linuxarm64/debug", "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" } },
		{ "name": "linux-arm64-release", "inherits": "linux-base", "binaryDir": "${sourceDir}/bin/linuxarm64/release", "cacheVariables": { "CMAKE_BUILD_TYPE": "Release" } },
		{ "name": "linux-arm64-debug-level-log", "inherits": "linux-base", "binaryDir": "${sourceDir}/bin/linuxarm64/debug_level_log", "cacheVariables": { "CMAKE_BUILD_TYPE": "DebugLevelLog" } },
		{ "name": "linux-arm64-release-level-log", "inherits": "linux-base", "binaryDir": "${sourceDir}/bin/linuxarm64/release_level_log", "cacheVariables": { "CMAKE_BUILD_TYPE": "ReleaseLevelLog" } },
		{
			"name": "windows-x64",
			"generator": "Visual Studio 18 2026",
			"architecture": { "value": "x64", "strategy": "set" },
			"binaryDir": "${sourceDir}/bin/windowsx64/vs",
			"condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows" },
			"cacheVariables": { "CMAKE_CONFIGURATION_TYPES": "Debug;Release;DebugLevelLog;ReleaseLevelLog" }
		}
	],
	"buildPresets": [
		{ "name": "linux-debug", "configurePreset": "linux-debug" },
		{ "name": "linux-release", "configurePreset": "linux-release" },
		{ "name": "linux-debug-level-log", "configurePreset": "linux-debug-level-log" },
		{ "name": "linux-release-level-log", "configurePreset": "linux-release-level-log" },
		{ "name": "linux-arm64-debug", "configurePreset": "linux-arm64-debug" },
		{ "name": "linux-arm64-release", "configurePreset": "linux-arm64-release" },
		{ "name": "linux-arm64-debug-level-log", "configurePreset": "linux-arm64-debug-level-log" },
		{ "name": "linux-arm64-release-level-log", "configurePreset": "linux-arm64-release-level-log" },
		{ "name": "windows-x64-debug", "configurePreset": "windows-x64", "configuration": "Debug" },
		{ "name": "windows-x64-release", "configurePreset": "windows-x64", "configuration": "Release" },
		{ "name": "windows-x64-debug-level-log", "configurePreset": "windows-x64", "configuration": "DebugLevelLog" },
		{ "name": "windows-x64-release-level-log", "configurePreset": "windows-x64", "configuration": "ReleaseLevelLog" }
	]
}
```

- [ ] **Step 5: Write `CMakeLists.txt`**

```cmake
cmake_minimum_required(VERSION 3.22)
project(Lightswitch VERSION 0.1.0 LANGUAGES CXX)

include(cmake/BuildLayout.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Network Test)
qt_standard_project_setup()

get_filename_component(LIGHTSWITCH_QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)

add_library(LightswitchCore STATIC
	src/core/SystemClock.cpp
	src/core/ClockModel.cpp
)
target_include_directories(LightswitchCore PUBLIC
	src/core
	src/weather
	src/calendar
	src/app
)
target_link_libraries(LightswitchCore PUBLIC Qt6::Core Qt6::Network)
target_compile_definitions(LightswitchCore PUBLIC ${LIGHTSWITCH_LOG_LEVEL_DEFINITION})

enable_testing()
add_subdirectory(tests)
```

Note for later tasks: each new logic class adds its `.cpp` to the `LightswitchCore` list, each new test adds one `add_lightswitch_test(...)` line.

- [ ] **Step 6: Write the test infrastructure**

`tests/CMakeLists.txt`:

```cmake
# Builds a QtTest executable from <name>.cpp and registers it with CTest.
function(add_lightswitch_test name)
	qt_add_executable(${name} ${name}.cpp)
	target_link_libraries(${name} PRIVATE LightswitchCore Qt6::Test)
	add_test(NAME ${name} COMMAND ${name})
	if(WIN32)
		set_tests_properties(${name} PROPERTIES
			ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${LIGHTSWITCH_QT_BIN_DIR}")
	endif()
endfunction()

add_lightswitch_test(ClockModelTest)
```

`tests/FakeClock.h`:

```cpp
#pragma once

#include "IClock.h"

// Clock whose current time is set explicitly by tests.
class FakeClock : public IClock {
public:
	explicit FakeClock(const QDateTime& now) : m_now(now) {}

	QDateTime Now() const override { return m_now; }

	void SetNow(const QDateTime& now) { m_now = now; }

private:
	QDateTime m_now;
};
```

`tests/ClockModelTest.cpp`:

```cpp
#include "ClockModel.h"
#include "FakeClock.h"

#include <QSignalSpy>
#include <QtTest>

class ClockModelTest : public QObject {
	Q_OBJECT

private slots:
	void FormatsTimeAndDate();
	void EmitsChangedOnlyWhenTextChanges();
};

void ClockModelTest::FormatsTimeAndDate() {
	FakeClock clock(QDateTime(QDate(2026, 9, 17), QTime(0, 24, 30)));
	ClockModel model(clock);

	QCOMPARE(model.TimeText(), QStringLiteral("00:24"));
	QCOMPARE(model.DateText(), QStringLiteral("Thursday sep 17"));
}

void ClockModelTest::EmitsChangedOnlyWhenTextChanges() {
	FakeClock clock(QDateTime(QDate(2026, 9, 17), QTime(0, 24, 0)));
	ClockModel model(clock);
	QSignalSpy spy(&model, &ClockModel::Changed);

	clock.SetNow(QDateTime(QDate(2026, 9, 17), QTime(0, 24, 30)));
	model.Refresh();
	QCOMPARE(spy.count(), 0);

	clock.SetNow(QDateTime(QDate(2026, 9, 17), QTime(0, 25, 0)));
	model.Refresh();
	QCOMPARE(spy.count(), 1);
	QCOMPARE(model.TimeText(), QStringLiteral("00:25"));
}

QTEST_GUILESS_MAIN(ClockModelTest)
#include "ClockModelTest.moc"
```

- [ ] **Step 7: Run the build to verify it fails**

Run the configure and build commands. Expected: FAIL, `ClockModel.h`/`IClock.h` not found (the sources listed in `CMakeLists.txt` do not exist yet, so configure itself already fails with "Cannot find source file").

- [ ] **Step 8: Implement the clock classes**

`src/core/IClock.h`:

```cpp
#pragma once

#include <QDateTime>

// Provides the current local date and time so that time dependent logic stays testable.
class IClock {
public:
	virtual ~IClock() = default;

	virtual QDateTime Now() const = 0;
};
```

`src/core/SystemClock.h`:

```cpp
#pragma once

#include "IClock.h"

// Clock backed by the operating system time.
class SystemClock : public IClock {
public:
	QDateTime Now() const override;
};
```

`src/core/SystemClock.cpp`:

```cpp
#include "SystemClock.h"

// Returns the current local time of the operating system.
QDateTime SystemClock::Now() const {
	return QDateTime::currentDateTime();
}
```

`src/core/ClockModel.h`:

```cpp
#pragma once

#include "IClock.h"

#include <QObject>
#include <QString>
#include <QTimer>

// Exposes the formatted time and date for display and refreshes them every second.
class ClockModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString timeText READ TimeText NOTIFY Changed)
	Q_PROPERTY(QString dateText READ DateText NOTIFY Changed)

public:
	explicit ClockModel(const IClock& clock, QObject* pParent = nullptr);

	const QString& TimeText() const;
	const QString& DateText() const;

	void Start();
	void Refresh();

signals:
	void Changed();

private:
	const IClock& m_clock;
	QTimer m_timer;
	QString m_timeText;
	QString m_dateText;
};
```

`src/core/ClockModel.cpp`:

```cpp
#include "ClockModel.h"

#include <QLocale>

namespace {
constexpr int refreshIntervalMilliseconds = 1000;
}

ClockModel::ClockModel(const IClock& clock, QObject* pParent)
	: QObject(pParent)
	, m_clock(clock) {
	m_timer.setInterval(refreshIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &ClockModel::Refresh);
	Refresh();
}

// Returns the time as HH:mm.
const QString& ClockModel::TimeText() const {
	return m_timeText;
}

// Returns the date such as "Thursday sep 17".
const QString& ClockModel::DateText() const {
	return m_dateText;
}

// Starts refreshing the texts periodically.
void ClockModel::Start() {
	m_timer.start();
}

// Re-reads the clock and notifies listeners when a displayed text has changed.
void ClockModel::Refresh() {
	const QDateTime now = m_clock.Now();
	const QDate date = now.date();
	const QLocale english(QLocale::English);

	const QString timeText = now.toString(QStringLiteral("HH:mm"));
	const QString dateText = english.toString(date, QStringLiteral("dddd")) + QLatin1Char(' ')
		+ english.toString(date, QStringLiteral("MMM")).toLower() + QLatin1Char(' ')
		+ QString::number(date.day());

	if (timeText == m_timeText && dateText == m_dateText) {
		return;
	}

	m_timeText = timeText;
	m_dateText = dateText;
	emit Changed();
}
```

- [ ] **Step 9: Run build and tests to verify they pass**

Expected: configure succeeds, `ClockModelTest` builds into `bin/windowsx64/debug/` (Windows) or `bin/linuxx64/debug/` (Linux) and `ctest` reports `100% tests passed, 0 tests failed out of 1`. Also confirm the output layout: `ls bin/*/` shows `debug/` containing `ClockModelTest(.exe)`.

- [ ] **Step 10: Write `BuildAndRun.sh`**

```bash
#!/usr/bin/env bash
# Standard Linux entry point: configure, build, test and run Lightswitch.
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APPLICATION_NAME="Lightswitch"

PROFILE=""
DO_CLEAN=0
DO_TEST=1
DO_RUN=1
DO_INSTALL_DEPS=0
DO_PULL=0
DO_RESET=0
JOBS="$(nproc 2>/dev/null || echo 4)"
APP_ARGS=()

print_usage() {
	cat <<'EOF'
Usage: ./BuildAndRun.sh [BuildProfile] [BuildOptions] [-- ApplicationArguments]

Build profiles (default: debug):
  debug
  release
  debug_level_log
  release_level_log

Build options:
  --clean          Delete the build folder of the selected profile before building.
  --no-test        Skip the automated tests.
  --no-run         Build and test, but do not start the application.
  --install-deps   Install missing build dependencies (apt based distributions).
  --pull           Run "git pull --ff-only" before building. Local changes are kept.
  --reset          DESTRUCTIVE: fetch the upstream branch and run "git reset --hard" to it,
                   discarding ALL local changes. Makes --pull unnecessary.
  -j N, --jobs N   Maximum number of parallel build processes.
  -h, --help       Show this help.

Application arguments:
  -- <arguments>   Everything after "--" is passed unchanged to the application.

Examples:
  ./BuildAndRun.sh release --clean --no-run -j 8
  ./BuildAndRun.sh debug_level_log -- --fullscreen
EOF
}

fail() {
	echo "Error: $*" >&2
	exit 1
}

set_profile() {
	[[ -z "$PROFILE" ]] || fail "Build profile specified more than once."
	PROFILE="$1"
}

install_dependencies() {
	command -v apt-get >/dev/null || fail "--install-deps supports apt based distributions only."
	sudo apt-get update
	sudo apt-get install -y build-essential cmake qt6-base-dev qt6-declarative-dev \
		qt6-declarative-dev-tools qml6-module-qtquick qml6-module-qtquick-window \
		qml6-module-qtqml-workerscript libgl1-mesa-dev
}

while [[ $# -gt 0 ]]; do
	case "$1" in
		debug|release|debug_level_log|release_level_log) set_profile "$1" ;;
		debug-level-log) set_profile debug_level_log ;;
		release-level-log) set_profile release_level_log ;;
		--clean) DO_CLEAN=1 ;;
		--no-test) DO_TEST=0 ;;
		--no-run) DO_RUN=0 ;;
		--install-deps) DO_INSTALL_DEPS=1 ;;
		--pull) DO_PULL=1 ;;
		--reset) DO_RESET=1 ;;
		-j|--jobs)
			[[ $# -ge 2 ]] || fail "$1 requires a number."
			[[ "$2" =~ ^[1-9][0-9]*$ ]] || fail "Invalid job count: $2"
			JOBS="$2"
			shift
			;;
		-h|--help) print_usage; exit 0 ;;
		--) shift; APP_ARGS=("$@"); break ;;
		*) fail "Unknown argument: $1 (see --help)" ;;
	esac
	shift
done

PROFILE="${PROFILE:-debug}"

case "$(uname -m)" in
	x86_64) ARCHITECTURE="x64"; PRESET_ARCHITECTURE="" ;;
	aarch64|arm64) ARCHITECTURE="arm64"; PRESET_ARCHITECTURE="arm64-" ;;
	*) fail "Unsupported architecture: $(uname -m)" ;;
esac

PRESET="linux-${PRESET_ARCHITECTURE}${PROFILE//_/-}"
BUILD_DIR="$SCRIPT_DIR/bin/linux${ARCHITECTURE}/${PROFILE}"
LOG_FILE="$BUILD_DIR/build.log"

cd "$SCRIPT_DIR"

if [[ $DO_INSTALL_DEPS -eq 1 ]]; then
	install_dependencies
fi

if [[ $DO_RESET -eq 1 ]]; then
	git fetch
	git reset --hard '@{u}'
fi

if [[ $DO_PULL -eq 1 ]]; then
	git pull --ff-only
fi

if [[ $DO_CLEAN -eq 1 && -d "$BUILD_DIR" ]]; then
	rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

if ! { cmake --preset "$PRESET" && cmake --build --preset "$PRESET" -j "$JOBS"; } 2>&1 | tee "$LOG_FILE"; then
	echo
	echo "Build failed. Errors found in $LOG_FILE:"
	grep -nE "error:|CMake Error|Error [0-9]+" "$LOG_FILE" | head -n 30 || true
	exit 1
fi

if [[ $DO_TEST -eq 1 ]]; then
	ctest --test-dir "$BUILD_DIR" --output-on-failure
fi

if [[ $DO_RUN -eq 1 ]]; then
	if [[ -x "$BUILD_DIR/$APPLICATION_NAME" ]]; then
		"$BUILD_DIR/$APPLICATION_NAME" ${APP_ARGS[@]+"${APP_ARGS[@]}"}
	else
		echo "No executable $APPLICATION_NAME in $BUILD_DIR, skipping start."
	fi
fi
```

Make it executable in git: `git update-index --chmod=+x BuildAndRun.sh` after `git add`.

- [ ] **Step 11: Write `CLAUDE.md` for the project**

```markdown
# Lightswitch

Qt 6 / C++17 light switch UI for a 720x720 display. Design spec: `docs/superpowers/specs/`, plan: `docs/superpowers/plans/`.

## Commands
- Linux: `./BuildAndRun.sh [debug|release|debug_level_log|release_level_log] [--clean] [--no-test] [--no-run] [-j N] [-- app args]`
- Windows (Developer PowerShell for VS 2026, Qt via `$env:CMAKE_PREFIX_PATH`): `cmake --preset windows-x64`, `cmake --build --preset windows-x64-debug`, `ctest --test-dir bin/windowsx64/vs -C Debug --output-on-failure`. Solution file: `bin/windowsx64/vs/Lightswitch.sln`.
- Application arguments: `--fullscreen`, `--config <file>` (default `lightswitch.ini` next to the executable).

## Layout
- `src/core` clock, light, alarm; `src/weather` Open-Meteo client and dot model; `src/calendar` calendar providers; `src/app` configuration, logging, `AppController`, `main.cpp`.
- `qml/` Qt Quick UI module `Lightswitch.Ui`; `resources/fonts` Manufaktur fonts; `config/lightswitch.ini` default configuration; `tests/` QtTest + CTest.
- Output always in `bin/<system><arch>/<config>/`. Log level is the compile definition `LIGHTSWITCH_LOG_LEVEL`, separate from the build configuration.

## Conventions
Global C++ standard applies (PascalCase methods/classes/files, `m_` members, camelCase variables, English ASCII comments, descriptions above `.cpp` methods and `.h` classes). New logic class: add its `.cpp` to `LightswitchCore` in `CMakeLists.txt` and a test via `add_lightswitch_test` in `tests/CMakeLists.txt`.
```

- [ ] **Step 12: Commit**

```bash
git add -A
git update-index --chmod=+x BuildAndRun.sh
git commit -m "feat: add build system foundation and clock model"
```

(append the Co-Authored-By trailer to the message)

---

### Task 2: Light controller

**Files:**
- Create: `src/core/ILightController.h`, `src/core/ILightController.cpp`, `src/core/DummyLightController.h`, `src/core/DummyLightController.cpp`
- Create: `tests/DummyLightControllerTest.cpp`
- Modify: `CMakeLists.txt` (add both `.cpp` to `LightswitchCore`), `tests/CMakeLists.txt` (add `add_lightswitch_test(DummyLightControllerTest)`)

**Interfaces:**
- Produces: `class ILightController : QObject` with `virtual bool IsOn() const = 0`, `virtual void SetOn(bool isOn) = 0`, `Q_INVOKABLE void Toggle()`, signal `StateChanged(bool isOn)`, property `isOn`. `class DummyLightController : ILightController` with `explicit DummyLightController(QObject* pParent = nullptr)`; it starts off and emits `StateChanged` only on change.

- [ ] **Step 1: Write the failing test** `tests/DummyLightControllerTest.cpp`

```cpp
#include "DummyLightController.h"

#include <QSignalSpy>
#include <QtTest>

class DummyLightControllerTest : public QObject {
	Q_OBJECT

private slots:
	void StartsOff();
	void SetOnEmitsOnlyOnChange();
	void ToggleFlipsState();
};

void DummyLightControllerTest::StartsOff() {
	DummyLightController light;

	QVERIFY(!light.IsOn());
}

void DummyLightControllerTest::SetOnEmitsOnlyOnChange() {
	DummyLightController light;
	QSignalSpy spy(&light, &ILightController::StateChanged);

	light.SetOn(true);
	light.SetOn(true);

	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.at(0).at(0).toBool(), true);
	QVERIFY(light.IsOn());
}

void DummyLightControllerTest::ToggleFlipsState() {
	DummyLightController light;

	light.Toggle();
	QVERIFY(light.IsOn());
	light.Toggle();
	QVERIFY(!light.IsOn());
}

QTEST_GUILESS_MAIN(DummyLightControllerTest)
#include "DummyLightControllerTest.moc"
```

- [ ] **Step 2: Register test and sources, run the build, verify it fails**

Add the test line and the two `.cpp` names (they do not exist yet). Expected: FAIL, source files missing / `DummyLightController.h` not found.

- [ ] **Step 3: Implement**

`src/core/ILightController.h`:

```cpp
#pragma once

#include <QObject>

// Abstract light switch so that the real hardware backend can be plugged in later.
class ILightController : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool isOn READ IsOn NOTIFY StateChanged)

public:
	explicit ILightController(QObject* pParent = nullptr);
	~ILightController() override = default;

	virtual bool IsOn() const = 0;
	virtual void SetOn(bool isOn) = 0;

	Q_INVOKABLE void Toggle();

signals:
	void StateChanged(bool isOn);
};
```

`src/core/ILightController.cpp`:

```cpp
#include "ILightController.h"

ILightController::ILightController(QObject* pParent)
	: QObject(pParent) {
}

// Switches the light to the opposite of its current state.
void ILightController::Toggle() {
	SetOn(!IsOn());
}
```

`src/core/DummyLightController.h`:

```cpp
#pragma once

#include "ILightController.h"

// Light controller that only remembers the state, used until real hardware is connected.
class DummyLightController : public ILightController {
	Q_OBJECT

public:
	explicit DummyLightController(QObject* pParent = nullptr);

	bool IsOn() const override;
	void SetOn(bool isOn) override;

private:
	bool m_isOn = false;
};
```

`src/core/DummyLightController.cpp`:

```cpp
#include "DummyLightController.h"

DummyLightController::DummyLightController(QObject* pParent)
	: ILightController(pParent) {
}

// Returns whether the simulated light is on.
bool DummyLightController::IsOn() const {
	return m_isOn;
}

// Stores the new state and notifies listeners when it changed.
void DummyLightController::SetOn(bool isOn) {
	if (m_isOn == isOn) {
		return;
	}

	m_isOn = isOn;
	emit StateChanged(m_isOn);
}
```

- [ ] **Step 4: Run build and tests to verify they pass**

Expected: `100% tests passed, 0 tests failed out of 2`.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat: add light controller interface and dummy implementation"
```

---

### Task 3: Alarm controller

**Files:**
- Create: `src/core/AlarmSettings.h`, `src/core/AlarmController.h`, `src/core/AlarmController.cpp`
- Create: `tests/AlarmControllerTest.cpp`
- Modify: `CMakeLists.txt` (add `src/core/AlarmController.cpp`), `tests/CMakeLists.txt` (add `add_lightswitch_test(AlarmControllerTest)`)

**Interfaces:**
- Consumes: `IClock::Now()`, `ILightController::SetOn(bool)`, `DummyLightController` (Tasks 1 and 2).
- Produces: `struct AlarmSettings { QTime time = QTime(6, 45); std::array<bool, 7> activeDays = {true, true, true, true, true, false, false}; }` (index 0 = Monday). `class AlarmController : QObject` with `AlarmController(const IClock& clock, ILightController& light, const AlarmSettings& settings, QObject* pParent = nullptr)`, `QString TimeText() const` ("HH:mm"), `QVariantList ActiveDays() const` (7 bools, Monday first), `void Start()`, `void Check()`, signal `Triggered()`, constant properties `timeText`, `activeDays`.

- [ ] **Step 1: Write the failing test** `tests/AlarmControllerTest.cpp`

```cpp
#include "AlarmController.h"
#include "DummyLightController.h"
#include "FakeClock.h"

#include <QSignalSpy>
#include <QtTest>

namespace {
const QDate wednesday(2026, 9, 30);
const QDate thursday(2026, 10, 1);
const QDate saturday(2026, 10, 3);

// Returns Monday to Friday at 06:45.
AlarmSettings MakeWeekdaySettings() {
	AlarmSettings settings;
	settings.time = QTime(6, 45);
	settings.activeDays = {true, true, true, true, true, false, false};
	return settings;
}

// Builds a local date time for the given day and time of day.
QDateTime At(const QDate& date, int hour, int minute, int second = 0) {
	return QDateTime(date, QTime(hour, minute, second));
}
}

class AlarmControllerTest : public QObject {
	Q_OBJECT

private slots:
	void TriggersOnActiveDayAtAlarmMinute();
	void IgnoresInactiveDays();
	void IgnoresOtherMinutes();
	void TriggersOnlyOncePerDay();
	void TriggersAgainOnNextActiveDay();
	void NeverSwitchesLightOff();
	void ExposesTimeTextAndActiveDays();
};

void AlarmControllerTest::TriggersOnActiveDayAtAlarmMinute() {
	FakeClock clock(At(wednesday, 6, 45, 10));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QSignalSpy spy(&alarm, &AlarmController::Triggered);

	alarm.Check();

	QVERIFY(light.IsOn());
	QCOMPARE(spy.count(), 1);
}

void AlarmControllerTest::IgnoresInactiveDays() {
	FakeClock clock(At(saturday, 6, 45));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());

	alarm.Check();

	QVERIFY(!light.IsOn());
}

void AlarmControllerTest::IgnoresOtherMinutes() {
	FakeClock clock(At(wednesday, 6, 44, 59));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());

	alarm.Check();
	QVERIFY(!light.IsOn());

	clock.SetNow(At(wednesday, 6, 46));
	alarm.Check();
	QVERIFY(!light.IsOn());
}

void AlarmControllerTest::TriggersOnlyOncePerDay() {
	FakeClock clock(At(wednesday, 6, 45, 0));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QSignalSpy spy(&alarm, &AlarmController::Triggered);

	alarm.Check();
	light.SetOn(false);
	clock.SetNow(At(wednesday, 6, 45, 30));
	alarm.Check();

	QVERIFY(!light.IsOn());
	QCOMPARE(spy.count(), 1);
}

void AlarmControllerTest::TriggersAgainOnNextActiveDay() {
	FakeClock clock(At(wednesday, 6, 45));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QSignalSpy spy(&alarm, &AlarmController::Triggered);

	alarm.Check();
	light.SetOn(false);
	clock.SetNow(At(thursday, 6, 45));
	alarm.Check();

	QVERIFY(light.IsOn());
	QCOMPARE(spy.count(), 2);
}

void AlarmControllerTest::NeverSwitchesLightOff() {
	FakeClock clock(At(wednesday, 6, 45));
	DummyLightController light;
	light.SetOn(true);
	AlarmController alarm(clock, light, MakeWeekdaySettings());

	alarm.Check();

	QVERIFY(light.IsOn());
}

void AlarmControllerTest::ExposesTimeTextAndActiveDays() {
	FakeClock clock(At(wednesday, 0, 0));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());

	QCOMPARE(alarm.TimeText(), QStringLiteral("06:45"));
	const QVariantList days = alarm.ActiveDays();
	QCOMPARE(days.size(), 7);
	QVERIFY(days.at(0).toBool());
	QVERIFY(days.at(4).toBool());
	QVERIFY(!days.at(5).toBool());
	QVERIFY(!days.at(6).toBool());
}

QTEST_GUILESS_MAIN(AlarmControllerTest)
#include "AlarmControllerTest.moc"
```

Dates: 2026-09-30 is a Wednesday, 2026-10-01 a Thursday, 2026-10-03 a Saturday.

- [ ] **Step 2: Register test and source, run the build, verify it fails**

Expected: FAIL, `AlarmController.h` not found.

- [ ] **Step 3: Implement**

`src/core/AlarmSettings.h`:

```cpp
#pragma once

#include <QTime>

#include <array>

// Alarm time and the weekdays (Monday first) on which the alarm is active.
struct AlarmSettings {
	QTime time = QTime(6, 45);
	std::array<bool, 7> activeDays = {true, true, true, true, true, false, false};
};
```

`src/core/AlarmController.h`:

```cpp
#pragma once

#include "AlarmSettings.h"
#include "IClock.h"
#include "ILightController.h"

#include <QDate>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>

// Switches the light on at the configured time on the configured weekdays, once per day.
class AlarmController : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString timeText READ TimeText CONSTANT)
	Q_PROPERTY(QVariantList activeDays READ ActiveDays CONSTANT)

public:
	AlarmController(const IClock& clock, ILightController& light, const AlarmSettings& settings, QObject* pParent = nullptr);

	QString TimeText() const;
	QVariantList ActiveDays() const;

	void Start();
	void Check();

signals:
	void Triggered();

private:
	bool IsActiveOn(const QDate& date) const;
	bool IsAlarmMinute(const QTime& time) const;

	const IClock& m_clock;
	ILightController& m_light;
	AlarmSettings m_settings;
	QDate m_lastTriggerDate;
	QTimer m_timer;
};
```

`src/core/AlarmController.cpp`:

```cpp
#include "AlarmController.h"

namespace {
constexpr int checkIntervalMilliseconds = 1000;
}

AlarmController::AlarmController(const IClock& clock, ILightController& light, const AlarmSettings& settings, QObject* pParent)
	: QObject(pParent)
	, m_clock(clock)
	, m_light(light)
	, m_settings(settings) {
	m_timer.setInterval(checkIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &AlarmController::Check);
}

// Returns the alarm time as HH:mm.
QString AlarmController::TimeText() const {
	return m_settings.time.toString(QStringLiteral("HH:mm"));
}

// Returns seven booleans, Monday first, telling on which weekdays the alarm is active.
QVariantList AlarmController::ActiveDays() const {
	QVariantList days;
	for (const bool isActive : m_settings.activeDays) {
		days.append(isActive);
	}
	return days;
}

// Starts checking the clock periodically.
void AlarmController::Start() {
	m_timer.start();
}

// Switches the light on when the alarm minute of an active day is reached for the first time that day.
void AlarmController::Check() {
	const QDateTime now = m_clock.Now();

	if (!IsActiveOn(now.date()) || !IsAlarmMinute(now.time())) {
		return;
	}

	if (m_lastTriggerDate == now.date()) {
		return;
	}

	m_lastTriggerDate = now.date();
	m_light.SetOn(true);
	emit Triggered();
}

// Returns whether the alarm is enabled for the weekday of the given date.
bool AlarmController::IsActiveOn(const QDate& date) const {
	return m_settings.activeDays.at(date.dayOfWeek() - 1);
}

// Returns whether the given time lies within the alarm minute.
bool AlarmController::IsAlarmMinute(const QTime& time) const {
	return time.hour() == m_settings.time.hour() && time.minute() == m_settings.time.minute();
}
```

- [ ] **Step 4: Run build and tests to verify they pass**

Expected: `100% tests passed, 0 tests failed out of 3`.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat: add alarm controller"
```

---

### Task 4: Configuration and log configuration

**Files:**
- Create: `src/app/Configuration.h`, `src/app/Configuration.cpp`, `src/app/LogConfiguration.h`, `src/app/LogConfiguration.cpp`, `config/lightswitch.ini`
- Create: `tests/ConfigurationTest.cpp`
- Modify: `CMakeLists.txt` (add `src/app/Configuration.cpp`, `src/app/LogConfiguration.cpp`), `tests/CMakeLists.txt` (add `add_lightswitch_test(ConfigurationTest)`)

**Interfaces:**
- Consumes: `AlarmSettings` (Task 3).
- Produces: `class Configuration` with `static Configuration Load(const QString& filePath)`, `static std::array<bool, 7> ParseActiveDays(const QString& text)`, `double Latitude() const`, `double Longitude() const`, `const AlarmSettings& Alarm() const`. Defaults: latitude 48.1374, longitude 11.5755, alarm 06:45 Monday to Friday. `class LogConfiguration { static void Apply(); }`.

- [ ] **Step 1: Write the failing test** `tests/ConfigurationTest.cpp`

```cpp
#include "Configuration.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

namespace {
// Writes an INI file with the given content into the directory and returns its path.
QString WriteIniFile(const QTemporaryDir& directory, const QByteArray& content) {
	const QString path = directory.filePath(QStringLiteral("lightswitch.ini"));
	QFile file(path);
	if (file.open(QIODevice::WriteOnly)) {
		file.write(content);
	}
	return path;
}

using Days = std::array<bool, 7>;
}

class ConfigurationTest : public QObject {
	Q_OBJECT

private slots:
	void LoadsValidFile();
	void MissingFileUsesDefaults();
	void EmptyFileUsesDefaults();
	void InvalidValuesUseDefaults();
	void ParsesActiveDays();
};

void ConfigurationTest::LoadsValidFile() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory,
		"[location]\nlatitude=52.52\nlongitude=13.405\n\n[alarm]\ntime=07:30\ndays=Mon,Wed,Sun\n");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Latitude(), 52.52);
	QCOMPARE(configuration.Longitude(), 13.405);
	QCOMPARE(configuration.Alarm().time, QTime(7, 30));
	QCOMPARE(configuration.Alarm().activeDays, (Days{true, false, true, false, false, false, true}));
}

void ConfigurationTest::MissingFileUsesDefaults() {
	QTemporaryDir directory;

	const Configuration configuration = Configuration::Load(directory.filePath(QStringLiteral("missing.ini")));

	QCOMPARE(configuration.Latitude(), 48.1374);
	QCOMPARE(configuration.Longitude(), 11.5755);
	QCOMPARE(configuration.Alarm().time, QTime(6, 45));
	QCOMPARE(configuration.Alarm().activeDays, (Days{true, true, true, true, true, false, false}));
}

void ConfigurationTest::EmptyFileUsesDefaults() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory, "");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Latitude(), 48.1374);
	QCOMPARE(configuration.Alarm().time, QTime(6, 45));
}

void ConfigurationTest::InvalidValuesUseDefaults() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory,
		"[location]\nlatitude=abc\nlongitude=500\n\n[alarm]\ntime=25:99\ndays=xyz\n");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Latitude(), 48.1374);
	QCOMPARE(configuration.Longitude(), 11.5755);
	QCOMPARE(configuration.Alarm().time, QTime(6, 45));
	QCOMPARE(configuration.Alarm().activeDays, (Days{false, false, false, false, false, false, false}));
}

void ConfigurationTest::ParsesActiveDays() {
	QCOMPARE(Configuration::ParseActiveDays(QStringLiteral(" mon , FRI, xyz")),
		(Days{true, false, false, false, true, false, false}));
	QCOMPARE(Configuration::ParseActiveDays(QStringLiteral("Saturday,Sunday")),
		(Days{false, false, false, false, false, true, true}));
	QCOMPARE(Configuration::ParseActiveDays(QString()),
		(Days{false, false, false, false, false, false, false}));
}

QTEST_GUILESS_MAIN(ConfigurationTest)
#include "ConfigurationTest.moc"
```

Design note: a present but unparsable `days` entry means "no active days" (alarm disabled), while an absent `days` key keeps the default Monday to Friday.

- [ ] **Step 2: Register test and sources, run the build, verify it fails**

Expected: FAIL, `Configuration.h` not found.

- [ ] **Step 3: Implement**

`src/app/Configuration.h`:

```cpp
#pragma once

#include "AlarmSettings.h"

#include <QString>

#include <array>

// Application settings loaded from an INI file, with defaults for missing or invalid entries.
class Configuration {
public:
	static Configuration Load(const QString& filePath);
	static std::array<bool, 7> ParseActiveDays(const QString& text);

	double Latitude() const;
	double Longitude() const;
	const AlarmSettings& Alarm() const;

private:
	double m_latitude = 48.1374;
	double m_longitude = 11.5755;
	AlarmSettings m_alarm;
};
```

`src/app/Configuration.cpp`:

```cpp
#include "Configuration.h"

#include <QSettings>
#include <QStringList>

namespace {
constexpr double maximumLatitude = 90.0;
constexpr double maximumLongitude = 180.0;

// Reads a coordinate within +-limit, or returns the fallback when it is missing or invalid.
double ReadCoordinate(const QSettings& settings, const QString& key, double fallback, double limit) {
	bool isNumber = false;
	const double value = settings.value(key).toString().toDouble(&isNumber);
	if (!isNumber || value < -limit || value > limit) {
		return fallback;
	}
	return value;
}
}

// Loads the configuration file and keeps the defaults for everything that is missing or invalid.
Configuration Configuration::Load(const QString& filePath) {
	const QSettings settings(filePath, QSettings::IniFormat);
	Configuration configuration;

	configuration.m_latitude = ReadCoordinate(settings, QStringLiteral("location/latitude"), configuration.m_latitude, maximumLatitude);
	configuration.m_longitude = ReadCoordinate(settings, QStringLiteral("location/longitude"), configuration.m_longitude, maximumLongitude);

	const QTime alarmTime = QTime::fromString(settings.value(QStringLiteral("alarm/time")).toString().trimmed(), QStringLiteral("HH:mm"));
	if (alarmTime.isValid()) {
		configuration.m_alarm.time = alarmTime;
	}

	if (settings.contains(QStringLiteral("alarm/days"))) {
		const QString daysText = settings.value(QStringLiteral("alarm/days")).toStringList().join(QLatin1Char(','));
		configuration.m_alarm.activeDays = ParseActiveDays(daysText);
	}

	return configuration;
}

// Parses a comma separated list of weekday names (first three letters count) into Monday-first flags.
std::array<bool, 7> Configuration::ParseActiveDays(const QString& text) {
	static const QStringList dayPrefixes = {
		QStringLiteral("mon"), QStringLiteral("tue"), QStringLiteral("wed"), QStringLiteral("thu"),
		QStringLiteral("fri"), QStringLiteral("sat"), QStringLiteral("sun")};

	std::array<bool, 7> activeDays = {};
	for (const QString& token : text.split(QLatin1Char(','))) {
		const int dayIndex = dayPrefixes.indexOf(token.trimmed().left(3).toLower());
		if (dayIndex >= 0) {
			activeDays.at(static_cast<size_t>(dayIndex)) = true;
		}
	}
	return activeDays;
}

// Returns the latitude of the weather location in degrees.
double Configuration::Latitude() const {
	return m_latitude;
}

// Returns the longitude of the weather location in degrees.
double Configuration::Longitude() const {
	return m_longitude;
}

// Returns the alarm time and active weekdays.
const AlarmSettings& Configuration::Alarm() const {
	return m_alarm;
}
```

`src/app/LogConfiguration.h`:

```cpp
#pragma once

// Applies the compile time log level (LIGHTSWITCH_LOG_LEVEL) to the Lightswitch logging categories.
class LogConfiguration {
public:
	static void Apply();
};
```

`src/app/LogConfiguration.cpp`:

```cpp
#include "LogConfiguration.h"

#include <QLoggingCategory>

#ifndef LIGHTSWITCH_LOG_LEVEL
#define LIGHTSWITCH_LOG_LEVEL 2
#endif

namespace {
constexpr int logLevel = LIGHTSWITCH_LOG_LEVEL;
constexpr int quietLevel = 0;
constexpr int verboseLevel = 3;
}

// Silences debug output for level 0 and enables it explicitly for level 3; level 2 keeps the Qt default.
void LogConfiguration::Apply() {
	if (logLevel <= quietLevel) {
		QLoggingCategory::setFilterRules(QStringLiteral("lightswitch.*.debug=false"));
	} else if (logLevel >= verboseLevel) {
		QLoggingCategory::setFilterRules(QStringLiteral("lightswitch.*.debug=true"));
	}
}
```

`config/lightswitch.ini`:

```ini
; Location used for the weather forecast (degrees).
[location]
latitude=48.1374
longitude=11.5755

; Alarm: switches the light on at this time (HH:mm) on the listed days.
[alarm]
time=06:45
days=Mon,Tue,Wed,Thu,Fri
```

- [ ] **Step 4: Run build and tests to verify they pass**

Expected: `100% tests passed, 0 tests failed out of 4`.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat: add configuration loading and log level configuration"
```

---

### Task 5: Open-Meteo parser

**Files:**
- Create: `src/weather/WeatherForecast.h`, `src/weather/OpenMeteoParser.h`, `src/weather/OpenMeteoParser.cpp`
- Create: `tests/OpenMeteoParserTest.cpp`
- Modify: `CMakeLists.txt` (add `src/weather/OpenMeteoParser.cpp`), `tests/CMakeLists.txt` (add `add_lightswitch_test(OpenMeteoParserTest)`)

**Interfaces:**
- Produces: `struct WeatherForecast { double temperatureCelsius = 0.0; QList<int> hourlyPrecipitationPercent; }` (list starts at the current hour, at most 24 entries). `class OpenMeteoParser` with `static std::optional<WeatherForecast> Parse(const QByteArray& json)` and `static QUrl BuildRequestUrl(double latitude, double longitude)`.

- [ ] **Step 1: Write the failing test** `tests/OpenMeteoParserTest.cpp`

```cpp
#include "OpenMeteoParser.h"

#include <QUrlQuery>
#include <QtTest>

namespace {
const QByteArray sampleResponse = R"({
	"current": { "time": "2026-09-30T01:20", "temperature_2m": 11.2 },
	"hourly": {
		"time": ["2026-09-30T00:00", "2026-09-30T01:00", "2026-09-30T02:00", "2026-09-30T03:00"],
		"precipitation_probability": [10, null, 30, 40]
	}
})";
}

class OpenMeteoParserTest : public QObject {
	Q_OBJECT

private slots:
	void ParsesTemperatureAndStartsAtCurrentHour();
	void TreatsNullProbabilityAsZero();
	void KeepsAtMostTwentyFourHours();
	void RejectsInvalidJson();
	void RejectsMissingCurrentBlock();
	void RejectsCurrentHourNotInList();
	void BuildsRequestUrl();
};

void OpenMeteoParserTest::ParsesTemperatureAndStartsAtCurrentHour() {
	const std::optional<WeatherForecast> forecast = OpenMeteoParser::Parse(sampleResponse);

	QVERIFY(forecast.has_value());
	QCOMPARE(forecast->temperatureCelsius, 11.2);
	QCOMPARE(forecast->hourlyPrecipitationPercent, (QList<int>{0, 30, 40}));
}

void OpenMeteoParserTest::TreatsNullProbabilityAsZero() {
	const std::optional<WeatherForecast> forecast = OpenMeteoParser::Parse(sampleResponse);

	QVERIFY(forecast.has_value());
	QCOMPARE(forecast->hourlyPrecipitationPercent.first(), 0);
}

void OpenMeteoParserTest::KeepsAtMostTwentyFourHours() {
	const QDateTime start(QDate(2026, 9, 30), QTime(0, 0));
	QStringList times;
	QStringList probabilities;
	for (int hour = 0; hour < 48; ++hour) {
		times << QLatin1Char('"') + start.addSecs(hour * 3600).toString(QStringLiteral("yyyy-MM-ddTHH:mm")) + QLatin1Char('"');
		probabilities << QString::number(hour);
	}
	const QByteArray json = QStringLiteral(
		R"({"current":{"time":"2026-09-30T00:15","temperature_2m":5.0},)"
		R"("hourly":{"time":[%1],"precipitation_probability":[%2]}})")
		.arg(times.join(QLatin1Char(',')), probabilities.join(QLatin1Char(','))).toUtf8();

	const std::optional<WeatherForecast> forecast = OpenMeteoParser::Parse(json);

	QVERIFY(forecast.has_value());
	QCOMPARE(forecast->hourlyPrecipitationPercent.size(), 24);
	QCOMPARE(forecast->hourlyPrecipitationPercent.last(), 23);
}

void OpenMeteoParserTest::RejectsInvalidJson() {
	QVERIFY(!OpenMeteoParser::Parse("not json").has_value());
	QVERIFY(!OpenMeteoParser::Parse("").has_value());
	QVERIFY(!OpenMeteoParser::Parse("[]").has_value());
}

void OpenMeteoParserTest::RejectsMissingCurrentBlock() {
	const QByteArray json = R"({"hourly":{"time":["2026-09-30T00:00"],"precipitation_probability":[10]}})";

	QVERIFY(!OpenMeteoParser::Parse(json).has_value());
}

void OpenMeteoParserTest::RejectsCurrentHourNotInList() {
	const QByteArray json = R"({
		"current": { "time": "2026-09-30T23:10", "temperature_2m": 11.2 },
		"hourly": { "time": ["2026-09-30T00:00"], "precipitation_probability": [10] }
	})";

	QVERIFY(!OpenMeteoParser::Parse(json).has_value());
}

void OpenMeteoParserTest::BuildsRequestUrl() {
	const QUrl url = OpenMeteoParser::BuildRequestUrl(48.1374, 11.5755);
	const QUrlQuery query(url);

	QCOMPARE(url.host(), QStringLiteral("api.open-meteo.com"));
	QCOMPARE(query.queryItemValue(QStringLiteral("latitude")), QStringLiteral("48.1374"));
	QCOMPARE(query.queryItemValue(QStringLiteral("longitude")), QStringLiteral("11.5755"));
	QCOMPARE(query.queryItemValue(QStringLiteral("current")), QStringLiteral("temperature_2m"));
	QCOMPARE(query.queryItemValue(QStringLiteral("hourly")), QStringLiteral("precipitation_probability"));
	QCOMPARE(query.queryItemValue(QStringLiteral("timezone")), QStringLiteral("auto"));
}

QTEST_GUILESS_MAIN(OpenMeteoParserTest)
#include "OpenMeteoParserTest.moc"
```

- [ ] **Step 2: Register test and source, run the build, verify it fails**

Expected: FAIL, `OpenMeteoParser.h` not found.

- [ ] **Step 3: Implement**

`src/weather/WeatherForecast.h`:

```cpp
#pragma once

#include <QList>

// Weather data needed by the weather tile: the current temperature and the hourly rain probability.
struct WeatherForecast {
	double temperatureCelsius = 0.0;
	QList<int> hourlyPrecipitationPercent;
};
```

`src/weather/OpenMeteoParser.h`:

```cpp
#pragma once

#include "WeatherForecast.h"

#include <QByteArray>
#include <QUrl>

#include <optional>

// Builds Open-Meteo requests and converts their JSON responses into WeatherForecast objects.
class OpenMeteoParser {
public:
	static std::optional<WeatherForecast> Parse(const QByteArray& json);
	static QUrl BuildRequestUrl(double latitude, double longitude);
};
```

`src/weather/OpenMeteoParser.cpp`:

```cpp
#include "OpenMeteoParser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

#include <algorithm>

namespace {
constexpr int forecastHourCount = 24;
constexpr int hourPrefixLength = 13; // Length of "yyyy-MM-ddTHH".
}

// Extracts the current temperature and the next 24 hourly rain probabilities, or nothing if the data is unusable.
std::optional<WeatherForecast> OpenMeteoParser::Parse(const QByteArray& json) {
	const QJsonDocument document = QJsonDocument::fromJson(json);
	if (!document.isObject()) {
		return std::nullopt;
	}

	const QJsonObject root = document.object();
	const QJsonObject current = root.value(QStringLiteral("current")).toObject();
	const QJsonObject hourly = root.value(QStringLiteral("hourly")).toObject();

	const QJsonValue temperature = current.value(QStringLiteral("temperature_2m"));
	const QString currentTime = current.value(QStringLiteral("time")).toString();
	const QJsonArray times = hourly.value(QStringLiteral("time")).toArray();
	const QJsonArray probabilities = hourly.value(QStringLiteral("precipitation_probability")).toArray();

	if (!temperature.isDouble() || currentTime.size() < hourPrefixLength || times.isEmpty() || times.size() != probabilities.size()) {
		return std::nullopt;
	}

	const QString currentHour = currentTime.left(hourPrefixLength);
	int startIndex = -1;
	for (int index = 0; index < times.size(); ++index) {
		if (times.at(index).toString().startsWith(currentHour)) {
			startIndex = index;
			break;
		}
	}
	if (startIndex < 0) {
		return std::nullopt;
	}

	WeatherForecast forecast;
	forecast.temperatureCelsius = temperature.toDouble();
	const int endIndex = std::min(startIndex + forecastHourCount, static_cast<int>(probabilities.size()));
	for (int index = startIndex; index < endIndex; ++index) {
		forecast.hourlyPrecipitationPercent.append(probabilities.at(index).toInt(0));
	}
	return forecast;
}

// Builds the forecast request URL for the given location in degrees.
QUrl OpenMeteoParser::BuildRequestUrl(double latitude, double longitude) {
	QUrlQuery query;
	query.addQueryItem(QStringLiteral("latitude"), QString::number(latitude, 'f', 4));
	query.addQueryItem(QStringLiteral("longitude"), QString::number(longitude, 'f', 4));
	query.addQueryItem(QStringLiteral("current"), QStringLiteral("temperature_2m"));
	query.addQueryItem(QStringLiteral("hourly"), QStringLiteral("precipitation_probability"));
	query.addQueryItem(QStringLiteral("forecast_days"), QStringLiteral("2"));
	query.addQueryItem(QStringLiteral("timezone"), QStringLiteral("auto"));

	QUrl url(QStringLiteral("https://api.open-meteo.com/v1/forecast"));
	url.setQuery(query);
	return url;
}
```

- [ ] **Step 4: Run build and tests to verify they pass**

Expected: `100% tests passed, 0 tests failed out of 5`.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat: add Open-Meteo response parser"
```

---

### Task 6: Weather model

**Files:**
- Create: `src/weather/WeatherModel.h`, `src/weather/WeatherModel.cpp`
- Create: `tests/WeatherModelTest.cpp`
- Modify: `CMakeLists.txt` (add `src/weather/WeatherModel.cpp`), `tests/CMakeLists.txt` (add `add_lightswitch_test(WeatherModelTest)`)

**Interfaces:**
- Consumes: `WeatherForecast` (Task 5).
- Produces: `class WeatherModel : QObject` with `static constexpr int columnCount = 12, hoursPerColumn = 2, maximumDotCount = 5`, `static QList<int> ComputeDotColumns(const QList<int>& hourlyPercent)`, `void SetForecast(const WeatherForecast& forecast)`, `bool HasData() const`, `QString TemperatureText() const` ("11" + degree sign, or "--"), `QString PrecipitationText() const` ("27%" or "--"), `QVariantList DotColumns() const` (12 ints 0..5), signal `Changed()`, properties `hasData`, `temperatureText`, `precipitationText`, `dotColumns`.

- [ ] **Step 1: Write the failing test** `tests/WeatherModelTest.cpp`

```cpp
#include "WeatherModel.h"

#include <QSignalSpy>
#include <QtTest>

class WeatherModelTest : public QObject {
	Q_OBJECT

private slots:
	void EmptyInputGivesZeroDots();
	void DotCountIsCeilOfPercentDividedByTwenty();
	void ColumnTakesMaximumOfItsTwoHours();
	void ValuesAreClamped();
	void ShortListLeavesRemainingColumnsEmpty();
	void InitialStateShowsPlaceholders();
	void SetForecastUpdatesTexts();
};

void WeatherModelTest::EmptyInputGivesZeroDots() {
	const QList<int> columns = WeatherModel::ComputeDotColumns({});

	QCOMPARE(columns, (QList<int>(12, 0)));
}

void WeatherModelTest::DotCountIsCeilOfPercentDividedByTwenty() {
	QCOMPARE(WeatherModel::ComputeDotColumns({0}).at(0), 0);
	QCOMPARE(WeatherModel::ComputeDotColumns({1}).at(0), 1);
	QCOMPARE(WeatherModel::ComputeDotColumns({20}).at(0), 1);
	QCOMPARE(WeatherModel::ComputeDotColumns({21}).at(0), 2);
	QCOMPARE(WeatherModel::ComputeDotColumns({80}).at(0), 4);
	QCOMPARE(WeatherModel::ComputeDotColumns({100}).at(0), 5);
}

void WeatherModelTest::ColumnTakesMaximumOfItsTwoHours() {
	const QList<int> columns = WeatherModel::ComputeDotColumns({0, 40, 100, 10});

	QCOMPARE(columns.at(0), 2);
	QCOMPARE(columns.at(1), 5);
}

void WeatherModelTest::ValuesAreClamped() {
	const QList<int> columns = WeatherModel::ComputeDotColumns({150, -20});

	QCOMPARE(columns.at(0), 5);
}

void WeatherModelTest::ShortListLeavesRemainingColumnsEmpty() {
	const QList<int> columns = WeatherModel::ComputeDotColumns({100, 100, 100});

	QCOMPARE(columns.size(), 12);
	QCOMPARE(columns.at(0), 5);
	QCOMPARE(columns.at(1), 5);
	QCOMPARE(columns.at(2), 0);
	QCOMPARE(columns.at(11), 0);
}

void WeatherModelTest::InitialStateShowsPlaceholders() {
	WeatherModel model;

	QVERIFY(!model.HasData());
	QCOMPARE(model.TemperatureText(), QStringLiteral("--"));
	QCOMPARE(model.PrecipitationText(), QStringLiteral("--"));
	QCOMPARE(model.DotColumns().size(), 12);
}

void WeatherModelTest::SetForecastUpdatesTexts() {
	WeatherModel model;
	QSignalSpy spy(&model, &WeatherModel::Changed);
	WeatherForecast forecast;
	forecast.temperatureCelsius = 11.4;
	forecast.hourlyPrecipitationPercent = {27, 0, 60};

	model.SetForecast(forecast);

	QVERIFY(model.HasData());
	QCOMPARE(model.TemperatureText(), QString::number(11) + QChar(0x00B0));
	QCOMPARE(model.PrecipitationText(), QStringLiteral("27%"));
	QCOMPARE(model.DotColumns().at(0).toInt(), 2);
	QCOMPARE(model.DotColumns().at(1).toInt(), 3);
	QCOMPARE(spy.count(), 1);
}

QTEST_GUILESS_MAIN(WeatherModelTest)
#include "WeatherModelTest.moc"
```

- [ ] **Step 2: Register test and source, run the build, verify it fails**

Expected: FAIL, `WeatherModel.h` not found.

- [ ] **Step 3: Implement**

`src/weather/WeatherModel.h`:

```cpp
#pragma once

#include "WeatherForecast.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>

// Turns a weather forecast into the texts and the 12x5 rain dot matrix shown on the weather tile.
class WeatherModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool hasData READ HasData NOTIFY Changed)
	Q_PROPERTY(QString temperatureText READ TemperatureText NOTIFY Changed)
	Q_PROPERTY(QString precipitationText READ PrecipitationText NOTIFY Changed)
	Q_PROPERTY(QVariantList dotColumns READ DotColumns NOTIFY Changed)

public:
	static constexpr int columnCount = 12;
	static constexpr int hoursPerColumn = 2;
	static constexpr int maximumDotCount = 5;

	static QList<int> ComputeDotColumns(const QList<int>& hourlyPercent);

	explicit WeatherModel(QObject* pParent = nullptr);

	bool HasData() const;
	QString TemperatureText() const;
	QString PrecipitationText() const;
	QVariantList DotColumns() const;

	void SetForecast(const WeatherForecast& forecast);

signals:
	void Changed();

private:
	bool m_hasData = false;
	QString m_temperatureText;
	QString m_precipitationText;
	QList<int> m_dotColumns;
};
```

`src/weather/WeatherModel.cpp`:

```cpp
#include "WeatherModel.h"

#include <algorithm>

namespace {
constexpr int maximumPercent = 100;
constexpr int percentPerDot = maximumPercent / WeatherModel::maximumDotCount;

// Returns the placeholder shown while no forecast is available.
QString Placeholder() {
	return QStringLiteral("--");
}
}

// Groups the hourly probabilities into 2 hour columns and converts each maximum into 0..5 dots.
QList<int> WeatherModel::ComputeDotColumns(const QList<int>& hourlyPercent) {
	QList<int> columns;
	const int availableHours = static_cast<int>(hourlyPercent.size());

	for (int column = 0; column < columnCount; ++column) {
		int columnMaximum = 0;
		for (int hour = 0; hour < hoursPerColumn; ++hour) {
			const int index = column * hoursPerColumn + hour;
			if (index < availableHours) {
				columnMaximum = std::max(columnMaximum, std::clamp(hourlyPercent.at(index), 0, maximumPercent));
			}
		}
		columns.append((columnMaximum + percentPerDot - 1) / percentPerDot);
	}
	return columns;
}

WeatherModel::WeatherModel(QObject* pParent)
	: QObject(pParent)
	, m_temperatureText(Placeholder())
	, m_precipitationText(Placeholder())
	, m_dotColumns(ComputeDotColumns({})) {
}

// Returns whether a forecast has been received yet.
bool WeatherModel::HasData() const {
	return m_hasData;
}

// Returns the rounded temperature with degree sign, or a placeholder.
QString WeatherModel::TemperatureText() const {
	return m_temperatureText;
}

// Returns the rain probability of the current hour, or a placeholder.
QString WeatherModel::PrecipitationText() const {
	return m_precipitationText;
}

// Returns the dot count of each of the 12 columns for the QML dot matrix.
QVariantList WeatherModel::DotColumns() const {
	QVariantList columns;
	for (const int dotCount : m_dotColumns) {
		columns.append(dotCount);
	}
	return columns;
}

// Replaces the displayed data with the given forecast.
void WeatherModel::SetForecast(const WeatherForecast& forecast) {
	m_hasData = true;
	m_temperatureText = QString::number(qRound(forecast.temperatureCelsius)) + QChar(0x00B0);
	m_precipitationText = forecast.hourlyPrecipitationPercent.isEmpty()
		? Placeholder()
		: QString::number(forecast.hourlyPrecipitationPercent.first()) + QLatin1Char('%');
	m_dotColumns = ComputeDotColumns(forecast.hourlyPrecipitationPercent);
	emit Changed();
}
```

`ComputeDotColumns({})` in the constructor needs `QList<int>` from braces; if the compiler reports ambiguity use `ComputeDotColumns(QList<int>())`.

- [ ] **Step 4: Run build and tests to verify they pass**

Expected: `100% tests passed, 0 tests failed out of 6`.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat: add weather model with rain dot matrix"
```

---

### Task 7: Weather service

**Files:**
- Create: `src/weather/WeatherService.h`, `src/weather/WeatherService.cpp`
- Modify: `CMakeLists.txt` (add `src/weather/WeatherService.cpp`)

The network code is a thin wrapper around `QNetworkAccessManager`; the logic (URL and parsing) is covered by `OpenMeteoParserTest`. Its behavior is verified against the live API in Task 10.

**Interfaces:**
- Consumes: `OpenMeteoParser::BuildRequestUrl`, `OpenMeteoParser::Parse`, `WeatherForecast` (Task 5).
- Produces: `class WeatherService : QObject` with `WeatherService(double latitude, double longitude, QObject* pParent = nullptr)`, `void Start()` (fetch now, then every 30 minutes), `void Refresh()`, signal `ForecastReady(const WeatherForecast& forecast)`.

- [ ] **Step 1: Implement**

`src/weather/WeatherService.h`:

```cpp
#pragma once

#include "WeatherForecast.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>

class QNetworkReply;

// Fetches the local forecast from Open-Meteo periodically and publishes it as WeatherForecast.
class WeatherService : public QObject {
	Q_OBJECT

public:
	WeatherService(double latitude, double longitude, QObject* pParent = nullptr);

	void Start();
	void Refresh();

signals:
	void ForecastReady(const WeatherForecast& forecast);

private:
	void HandleReply(QNetworkReply* pReply);

	QNetworkAccessManager m_network;
	QTimer m_refreshTimer;
	QUrl m_requestUrl;
};
```

`src/weather/WeatherService.cpp`:

```cpp
#include "WeatherService.h"

#include "OpenMeteoParser.h"

#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>

Q_LOGGING_CATEGORY(lcWeather, "lightswitch.weather")

namespace {
constexpr int refreshIntervalMilliseconds = 30 * 60 * 1000;
constexpr int requestTimeoutMilliseconds = 15 * 1000;
}

WeatherService::WeatherService(double latitude, double longitude, QObject* pParent)
	: QObject(pParent)
	, m_requestUrl(OpenMeteoParser::BuildRequestUrl(latitude, longitude)) {
	m_refreshTimer.setInterval(refreshIntervalMilliseconds);
	connect(&m_refreshTimer, &QTimer::timeout, this, &WeatherService::Refresh);
}

// Fetches the forecast immediately and then keeps refreshing it.
void WeatherService::Start() {
	Refresh();
	m_refreshTimer.start();
}

// Requests a fresh forecast; failures are logged and the previous data stays visible.
void WeatherService::Refresh() {
	QNetworkRequest request(m_requestUrl);
	request.setTransferTimeout(requestTimeoutMilliseconds);

	QNetworkReply* pReply = m_network.get(request);
	connect(pReply, &QNetworkReply::finished, this, [this, pReply]() { HandleReply(pReply); });
	qCDebug(lcWeather) << "Requesting forecast" << m_requestUrl;
}

// Converts a finished reply into a forecast or logs why that was not possible.
void WeatherService::HandleReply(QNetworkReply* pReply) {
	pReply->deleteLater();

	if (pReply->error() != QNetworkReply::NoError) {
		qCWarning(lcWeather) << "Forecast request failed:" << pReply->errorString();
		return;
	}

	const std::optional<WeatherForecast> forecast = OpenMeteoParser::Parse(pReply->readAll());
	if (!forecast.has_value()) {
		qCWarning(lcWeather) << "Forecast response could not be parsed.";
		return;
	}

	qCDebug(lcWeather) << "Forecast received, temperature" << forecast->temperatureCelsius;
	emit ForecastReady(*forecast);
}
```

- [ ] **Step 2: Run the build and the tests**

Expected: library compiles, `100% tests passed, 0 tests failed out of 6`.

- [ ] **Step 3: Commit**

```bash
git add -A
git commit -m "feat: add Open-Meteo weather service"
```

---

### Task 8: Calendar providers and model

**Files:**
- Create: `src/calendar/CalendarEvent.h`, `src/calendar/ICalendarProvider.h`, `src/calendar/DummyCalendarProvider.h`, `src/calendar/DummyCalendarProvider.cpp`, `src/calendar/CalendarModel.h`, `src/calendar/CalendarModel.cpp`
- Create: `tests/DummyCalendarProviderTest.cpp`, `tests/CalendarModelTest.cpp`
- Modify: `CMakeLists.txt` (add both `.cpp`), `tests/CMakeLists.txt` (add both tests)

**Interfaces:**
- Consumes: `IClock` (Task 1), `FakeClock` (Task 1).
- Produces: `struct CalendarEvent { QString title; QDateTime start; }`. `class ICalendarProvider { virtual QList<CalendarEvent> UpcomingEvents(const QDateTime& from) const = 0; }` (sorted ascending, all starting at or after `from`). `class DummyCalendarProvider : ICalendarProvider` with `explicit DummyCalendarProvider(quint32 seed)`, deterministic per seed and date, 1 to 2 events per day between 08:00 and 20:45 for the next 7 days. `class CalendarModel : QObject` with `CalendarModel(const ICalendarProvider& provider, const IClock& clock, QObject* pParent = nullptr)`, `const QString& TitleText() const` (next event title or "No events"), `const QString& WhenText() const` ("dd.MM.yyyy" + bullet `QChar(0x2022)` + "HH:mm", empty when no event), `void Start()` (refresh every 60 s), `void Refresh()`, signal `Changed()`, properties `titleText`, `whenText`.

- [ ] **Step 1: Write the failing tests**

`tests/DummyCalendarProviderTest.cpp`:

```cpp
#include "DummyCalendarProvider.h"

#include <QtTest>

namespace {
const QDateTime from(QDate(2026, 9, 30), QTime(12, 0));
}

class DummyCalendarProviderTest : public QObject {
	Q_OBJECT

private slots:
	void IsDeterministicForSameSeed();
	void ReturnsSortedEventsNotBeforeFrom();
	void EventsLieWithinDaytime();
	void ProvidesEventsOnEveryUpcomingDay();
};

void DummyCalendarProviderTest::IsDeterministicForSameSeed() {
	const DummyCalendarProvider first(42);
	const DummyCalendarProvider second(42);

	const QList<CalendarEvent> firstEvents = first.UpcomingEvents(from);
	const QList<CalendarEvent> secondEvents = second.UpcomingEvents(from);

	QCOMPARE(firstEvents.size(), secondEvents.size());
	for (int index = 0; index < firstEvents.size(); ++index) {
		QCOMPARE(firstEvents.at(index).title, secondEvents.at(index).title);
		QCOMPARE(firstEvents.at(index).start, secondEvents.at(index).start);
	}
}

void DummyCalendarProviderTest::ReturnsSortedEventsNotBeforeFrom() {
	const DummyCalendarProvider provider(7);

	const QList<CalendarEvent> events = provider.UpcomingEvents(from);

	QVERIFY(!events.isEmpty());
	for (int index = 0; index < events.size(); ++index) {
		QVERIFY(events.at(index).start >= from);
		if (index > 0) {
			QVERIFY(events.at(index - 1).start <= events.at(index).start);
		}
	}
}

void DummyCalendarProviderTest::EventsLieWithinDaytime() {
	const DummyCalendarProvider provider(99);

	for (const CalendarEvent& event : provider.UpcomingEvents(from)) {
		QVERIFY(!event.title.isEmpty());
		QVERIFY(event.start.time().hour() >= 8);
		QVERIFY(event.start.time().hour() <= 20);
	}
}

void DummyCalendarProviderTest::ProvidesEventsOnEveryUpcomingDay() {
	const DummyCalendarProvider provider(3);
	const QDateTime startOfDay(QDate(2026, 9, 30), QTime(0, 0));

	QSet<QDate> days;
	for (const CalendarEvent& event : provider.UpcomingEvents(startOfDay)) {
		days.insert(event.start.date());
	}

	QCOMPARE(days.size(), 7);
}

QTEST_GUILESS_MAIN(DummyCalendarProviderTest)
#include "DummyCalendarProviderTest.moc"
```

`tests/CalendarModelTest.cpp`:

```cpp
#include "CalendarModel.h"
#include "FakeClock.h"

#include <QSignalSpy>
#include <QtTest>

namespace {
// Provider returning a fixed list of events.
class StubCalendarProvider : public ICalendarProvider {
public:
	explicit StubCalendarProvider(const QList<CalendarEvent>& events) : m_events(events) {}

	QList<CalendarEvent> UpcomingEvents(const QDateTime&) const override { return m_events; }

	void SetEvents(const QList<CalendarEvent>& events) { m_events = events; }

private:
	QList<CalendarEvent> m_events;
};
}

class CalendarModelTest : public QObject {
	Q_OBJECT

private slots:
	void ShowsNextEvent();
	void ShowsPlaceholderWithoutEvents();
	void RefreshUpdatesAndEmitsChanged();
};

void CalendarModelTest::ShowsNextEvent() {
	FakeClock clock(QDateTime(QDate(2026, 9, 30), QTime(12, 0)));
	StubCalendarProvider provider({{QStringLiteral("Dentist"), QDateTime(QDate(2026, 10, 1), QTime(13, 30))}});

	const CalendarModel model(provider, clock);

	QCOMPARE(model.TitleText(), QStringLiteral("Dentist"));
	QCOMPARE(model.WhenText(), QStringLiteral("01.10.2026") + QChar(0x2022) + QStringLiteral("13:30"));
}

void CalendarModelTest::ShowsPlaceholderWithoutEvents() {
	FakeClock clock(QDateTime(QDate(2026, 9, 30), QTime(12, 0)));
	StubCalendarProvider provider({});

	const CalendarModel model(provider, clock);

	QCOMPARE(model.TitleText(), QStringLiteral("No events"));
	QVERIFY(model.WhenText().isEmpty());
}

void CalendarModelTest::RefreshUpdatesAndEmitsChanged() {
	FakeClock clock(QDateTime(QDate(2026, 9, 30), QTime(12, 0)));
	StubCalendarProvider provider({});
	CalendarModel model(provider, clock);
	QSignalSpy spy(&model, &CalendarModel::Changed);

	model.Refresh();
	QCOMPARE(spy.count(), 0);

	provider.SetEvents({{QStringLiteral("Gym"), QDateTime(QDate(2026, 9, 30), QTime(18, 0))}});
	model.Refresh();
	QCOMPARE(spy.count(), 1);
	QCOMPARE(model.TitleText(), QStringLiteral("Gym"));
}

QTEST_GUILESS_MAIN(CalendarModelTest)
#include "CalendarModelTest.moc"
```

- [ ] **Step 2: Register tests and sources, run the build, verify it fails**

Expected: FAIL, `DummyCalendarProvider.h` / `CalendarModel.h` not found.

- [ ] **Step 3: Implement**

`src/calendar/CalendarEvent.h`:

```cpp
#pragma once

#include <QDateTime>
#include <QString>

// A single calendar entry with its title and start time.
struct CalendarEvent {
	QString title;
	QDateTime start;
};
```

`src/calendar/ICalendarProvider.h`:

```cpp
#pragma once

#include "CalendarEvent.h"

#include <QList>

// Source of calendar events, implemented by the dummy provider now and by Google Calendar later.
class ICalendarProvider {
public:
	virtual ~ICalendarProvider() = default;

	virtual QList<CalendarEvent> UpcomingEvents(const QDateTime& from) const = 0;
};
```

`src/calendar/DummyCalendarProvider.h`:

```cpp
#pragma once

#include "ICalendarProvider.h"

#include <QtGlobal>

// Generates reproducible random events for the next days, used until a real calendar is connected.
class DummyCalendarProvider : public ICalendarProvider {
public:
	explicit DummyCalendarProvider(quint32 seed);

	QList<CalendarEvent> UpcomingEvents(const QDateTime& from) const override;

private:
	QList<CalendarEvent> EventsForDate(const QDate& date) const;

	quint32 m_seed;
};
```

`src/calendar/DummyCalendarProvider.cpp`:

```cpp
#include "DummyCalendarProvider.h"

#include <QRandomGenerator>
#include <QStringList>

#include <algorithm>

namespace {
constexpr int lookAheadDays = 7;
constexpr int firstEventHour = 8;
constexpr int lastEventHour = 20;
constexpr int minutesPerQuarterHour = 15;
constexpr int quarterHoursPerHour = 4;
constexpr int maximumEventsPerDay = 2;
}

DummyCalendarProvider::DummyCalendarProvider(quint32 seed)
	: m_seed(seed) {
}

// Returns the generated events of the next seven days that start at or after the given time.
QList<CalendarEvent> DummyCalendarProvider::UpcomingEvents(const QDateTime& from) const {
	QList<CalendarEvent> events;
	for (int dayOffset = 0; dayOffset < lookAheadDays; ++dayOffset) {
		for (const CalendarEvent& event : EventsForDate(from.date().addDays(dayOffset))) {
			if (event.start >= from) {
				events.append(event);
			}
		}
	}
	return events;
}

// Generates one or two sorted events for the date, derived only from the seed and the date.
QList<CalendarEvent> DummyCalendarProvider::EventsForDate(const QDate& date) const {
	static const QStringList titles = {
		QStringLiteral("Dinner with Dad"), QStringLiteral("Team meeting"), QStringLiteral("Dentist"),
		QStringLiteral("Call with Anna"), QStringLiteral("Grocery shopping"), QStringLiteral("Gym")};

	QRandomGenerator generator(m_seed + static_cast<quint32>(date.toJulianDay()));
	const int eventCount = generator.bounded(1, maximumEventsPerDay + 1);

	QList<CalendarEvent> events;
	for (int index = 0; index < eventCount; ++index) {
		CalendarEvent event;
		event.title = titles.at(generator.bounded(static_cast<int>(titles.size())));
		const int hour = generator.bounded(firstEventHour, lastEventHour + 1);
		const int minute = generator.bounded(quarterHoursPerHour) * minutesPerQuarterHour;
		event.start = QDateTime(date, QTime(hour, minute));
		events.append(event);
	}

	std::sort(events.begin(), events.end(), [](const CalendarEvent& left, const CalendarEvent& right) {
		return left.start < right.start;
	});
	return events;
}
```

`src/calendar/CalendarModel.h`:

```cpp
#pragma once

#include "IClock.h"
#include "ICalendarProvider.h"

#include <QObject>
#include <QString>
#include <QTimer>

// Exposes the next upcoming calendar event as text for the calendar tile.
class CalendarModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString titleText READ TitleText NOTIFY Changed)
	Q_PROPERTY(QString whenText READ WhenText NOTIFY Changed)

public:
	CalendarModel(const ICalendarProvider& provider, const IClock& clock, QObject* pParent = nullptr);

	const QString& TitleText() const;
	const QString& WhenText() const;

	void Start();
	void Refresh();

signals:
	void Changed();

private:
	const ICalendarProvider& m_provider;
	const IClock& m_clock;
	QTimer m_timer;
	QString m_titleText;
	QString m_whenText;
};
```

`src/calendar/CalendarModel.cpp`:

```cpp
#include "CalendarModel.h"

namespace {
constexpr int refreshIntervalMilliseconds = 60 * 1000;
}

CalendarModel::CalendarModel(const ICalendarProvider& provider, const IClock& clock, QObject* pParent)
	: QObject(pParent)
	, m_provider(provider)
	, m_clock(clock) {
	m_timer.setInterval(refreshIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &CalendarModel::Refresh);
	Refresh();
}

// Returns the title of the next event, or "No events".
const QString& CalendarModel::TitleText() const {
	return m_titleText;
}

// Returns the date and time of the next event, or an empty text.
const QString& CalendarModel::WhenText() const {
	return m_whenText;
}

// Starts refreshing the next event periodically.
void CalendarModel::Start() {
	m_timer.start();
}

// Asks the provider for the next event and notifies listeners when the displayed texts changed.
void CalendarModel::Refresh() {
	const QList<CalendarEvent> events = m_provider.UpcomingEvents(m_clock.Now());

	QString titleText = QStringLiteral("No events");
	QString whenText;
	if (!events.isEmpty()) {
		const CalendarEvent& nextEvent = events.first();
		titleText = nextEvent.title;
		whenText = nextEvent.start.toString(QStringLiteral("dd.MM.yyyy")) + QChar(0x2022)
			+ nextEvent.start.toString(QStringLiteral("HH:mm"));
	}

	if (titleText == m_titleText && whenText == m_whenText) {
		return;
	}

	m_titleText = titleText;
	m_whenText = whenText;
	emit Changed();
}
```

In `CalendarModelTest.cpp` the test declares `const CalendarModel model(...)` in two tests but calls `Refresh()` (non-const) in the third; the third declares it non-const already. Keep as written.

- [ ] **Step 4: Run build and tests to verify they pass**

Expected: `100% tests passed, 0 tests failed out of 8`.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat: add calendar providers and calendar model"
```

---

### Task 9: Application wiring and Qt Quick UI

**Files:**
- Create: `src/app/AppController.h`, `src/app/AppController.cpp`, `src/app/main.cpp`
- Create: `qml/CMakeLists.txt`, `qml/Theme.qml`, `qml/Tile.qml`, `qml/TileText.qml`, `qml/DotMatrix.qml`, `qml/TimeTile.qml`, `qml/LightTile.qml`, `qml/WeatherTile.qml`, `qml/CalendarTile.qml`, `qml/AlarmTile.qml`, `qml/Main.qml`
- Modify: `CMakeLists.txt` (more Qt components, `AppController.cpp` in core library, executable, QML subdirectory, post-build steps)

**Interfaces:**
- Consumes: all classes from Tasks 1 to 8.
- Produces: `class AppController : QObject` with `AppController(const Configuration& configuration, bool isFullscreen, QObject* pParent = nullptr)`, constant properties `clock`, `light`, `alarm`, `weather`, `calendar` (each `QObject*`) and `isFullscreen` (bool), `void Start()`. QML module `Lightswitch.Ui` (resource path `qrc:/qt/qml/Lightswitch/Ui/`) with root `Main.qml`; context property `app`. Executable target `Lightswitch` with options `--fullscreen` and `--config <file>`.

- [ ] **Step 1: Implement `AppController`**

`src/app/AppController.h`:

```cpp
#pragma once

#include "AlarmController.h"
#include "CalendarModel.h"
#include "ClockModel.h"
#include "Configuration.h"
#include "DummyCalendarProvider.h"
#include "DummyLightController.h"
#include "SystemClock.h"
#include "WeatherModel.h"
#include "WeatherService.h"

#include <QObject>

// Owns and wires all models and services and exposes them to the QML user interface.
class AppController : public QObject {
	Q_OBJECT
	Q_PROPERTY(QObject* clock READ Clock CONSTANT)
	Q_PROPERTY(QObject* light READ Light CONSTANT)
	Q_PROPERTY(QObject* alarm READ Alarm CONSTANT)
	Q_PROPERTY(QObject* weather READ Weather CONSTANT)
	Q_PROPERTY(QObject* calendar READ Calendar CONSTANT)
	Q_PROPERTY(bool isFullscreen READ IsFullscreen CONSTANT)

public:
	AppController(const Configuration& configuration, bool isFullscreen, QObject* pParent = nullptr);

	QObject* Clock();
	QObject* Light();
	QObject* Alarm();
	QObject* Weather();
	QObject* Calendar();
	bool IsFullscreen() const;

	void Start();

private:
	SystemClock m_systemClock;
	DummyLightController m_light;
	ClockModel m_clockModel;
	AlarmController m_alarmController;
	WeatherModel m_weatherModel;
	WeatherService m_weatherService;
	DummyCalendarProvider m_calendarProvider;
	CalendarModel m_calendarModel;
	bool m_isFullscreen;
};
```

`src/app/AppController.cpp`:

```cpp
#include "AppController.h"

#include <QRandomGenerator>

AppController::AppController(const Configuration& configuration, bool isFullscreen, QObject* pParent)
	: QObject(pParent)
	, m_clockModel(m_systemClock)
	, m_alarmController(m_systemClock, m_light, configuration.Alarm())
	, m_weatherService(configuration.Latitude(), configuration.Longitude())
	, m_calendarProvider(QRandomGenerator::global()->generate())
	, m_calendarModel(m_calendarProvider, m_systemClock)
	, m_isFullscreen(isFullscreen) {
	connect(&m_weatherService, &WeatherService::ForecastReady, &m_weatherModel, &WeatherModel::SetForecast);
}

// Returns the clock model for QML.
QObject* AppController::Clock() {
	return &m_clockModel;
}

// Returns the light controller for QML.
QObject* AppController::Light() {
	return &m_light;
}

// Returns the alarm controller for QML.
QObject* AppController::Alarm() {
	return &m_alarmController;
}

// Returns the weather model for QML.
QObject* AppController::Weather() {
	return &m_weatherModel;
}

// Returns the calendar model for QML.
QObject* AppController::Calendar() {
	return &m_calendarModel;
}

// Returns whether the window should cover the whole screen.
bool AppController::IsFullscreen() const {
	return m_isFullscreen;
}

// Starts all periodic updates.
void AppController::Start() {
	m_clockModel.Start();
	m_alarmController.Start();
	m_calendarModel.Start();
	m_weatherService.Start();
}
```

`src/app/main.cpp`:

```cpp
#include "AppController.h"
#include "Configuration.h"
#include "LogConfiguration.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[]) {
	QGuiApplication application(argc, argv);
	QGuiApplication::setApplicationName(QStringLiteral("Lightswitch"));

	const QString defaultConfigPath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("lightswitch.ini"));

	QCommandLineParser parser;
	parser.addHelpOption();
	const QCommandLineOption fullscreenOption(QStringLiteral("fullscreen"), QStringLiteral("Show the window in fullscreen mode."));
	const QCommandLineOption configOption(QStringLiteral("config"), QStringLiteral("Path to the configuration file."), QStringLiteral("file"), defaultConfigPath);
	parser.addOption(fullscreenOption);
	parser.addOption(configOption);
	parser.process(application);

	LogConfiguration::Apply();

	const Configuration configuration = Configuration::Load(parser.value(configOption));
	AppController controller(configuration, parser.isSet(fullscreenOption));

	QQmlApplicationEngine engine;
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &application,
		[]() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
	engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
	engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Lightswitch/Ui/Main.qml")));

	controller.Start();
	return application.exec();
}
```

- [ ] **Step 2: Extend the top-level `CMakeLists.txt`**

Change the `find_package` line and add `src/app/AppController.cpp` to the `LightswitchCore` source list, then append the executable section after `add_subdirectory(tests)`:

```cmake
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Network Gui Qml Quick Test)
```

```cmake
add_subdirectory(qml)

qt_add_executable(Lightswitch src/app/main.cpp)
target_link_libraries(Lightswitch PRIVATE
	LightswitchCore
	LightswitchUi
	LightswitchUiplugin
	Qt6::Gui
	Qt6::Qml
	Qt6::Quick
)

add_custom_command(TARGET Lightswitch POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_SOURCE_DIR}/config/lightswitch.ini
		$<TARGET_FILE_DIR:Lightswitch>/lightswitch.ini
)

if(WIN32)
	find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${LIGHTSWITCH_QT_BIN_DIR}" REQUIRED)
	add_custom_command(TARGET Lightswitch POST_BUILD
		COMMAND ${WINDEPLOYQT_EXECUTABLE} --no-translations --qmldir ${CMAKE_SOURCE_DIR}/qml $<TARGET_FILE:Lightswitch>
	)
endif()
```

`qml/CMakeLists.txt`:

```cmake
set(LIGHTSWITCH_FONT_DIR ${CMAKE_SOURCE_DIR}/resources/fonts)

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(${LIGHTSWITCH_FONT_DIR}/Manufaktur-Bold.ttf PROPERTIES QT_RESOURCE_ALIAS fonts/Manufaktur-Bold.ttf)
set_source_files_properties(${LIGHTSWITCH_FONT_DIR}/Manufaktur-Black.ttf PROPERTIES QT_RESOURCE_ALIAS fonts/Manufaktur-Black.ttf)

qt_add_library(LightswitchUi STATIC)
qt_add_qml_module(LightswitchUi
	URI Lightswitch.Ui
	VERSION 1.0
	RESOURCE_PREFIX /qt/qml
	QML_FILES
		Main.qml
		Theme.qml
		Tile.qml
		TileText.qml
		DotMatrix.qml
		TimeTile.qml
		LightTile.qml
		WeatherTile.qml
		CalendarTile.qml
		AlarmTile.qml
	RESOURCES
		${LIGHTSWITCH_FONT_DIR}/Manufaktur-Bold.ttf
		${LIGHTSWITCH_FONT_DIR}/Manufaktur-Black.ttf
)
target_link_libraries(LightswitchUi PRIVATE Qt6::Quick)
```

- [ ] **Step 3: Write the QML design system**

Coordinates below are derived from `Lightswitch.svg` and are local to each tile (tile origin = top-left of its rectangle; text is positioned by baseline).

`qml/Theme.qml`:

```qml
pragma Singleton
import QtQuick

QtObject {
	readonly property color accent: "#CA6F54"
	readonly property color label: "#999999"
	readonly property color value: "#EDEDED"
	readonly property color tileFill: "#241F23"
	readonly property color tileBorder: "#2D2B2E"
	readonly property color lightTileFill: "#1F493B"
	readonly property color lightTileBorder: "#3C4E50"
	readonly property color emptyDot: "#2D2B2E"

	readonly property real tileRadius: 15
	readonly property int labelSize: 21
	readonly property int valueSize: 42
	readonly property int heroSize: 133

	readonly property FontLoader boldLoader: FontLoader { source: "fonts/Manufaktur-Bold.ttf" }
	readonly property FontLoader blackLoader: FontLoader { source: "fonts/Manufaktur-Black.ttf" }
	readonly property string boldFamily: boldLoader.name
	readonly property string blackFamily: blackLoader.name
}
```

`qml/Tile.qml`:

```qml
import QtQuick

Rectangle {
	property string title: ""
	property bool isHighlighted: false

	radius: Theme.tileRadius
	color: isHighlighted ? Theme.lightTileFill : Theme.tileFill
	border.width: 2
	border.color: isHighlighted ? Theme.lightTileBorder : Theme.tileBorder

	TileText {
		x: 14
		baselineY: 29.6
		text: parent.title
	}
}
```

`qml/TileText.qml`:

```qml
import QtQuick

Text {
	property real baselineY: 0
	property bool isBlack: false

	y: baselineY - baselineOffset
	color: Theme.label
	font.family: isBlack ? Theme.blackFamily : Theme.boldFamily
	font.pixelSize: Theme.labelSize
}
```

`qml/DotMatrix.qml`:

```qml
import QtQuick

// 12 columns x 5 rows of dots; the item origin is the center of the bottom-left dot.
Item {
	property var columns: []
	readonly property int columnCount: 12
	readonly property int rowCount: 5
	readonly property real pitch: 16.2857
	readonly property real dotRadius: 5.43

	Repeater {
		model: parent.columnCount * parent.rowCount

		Rectangle {
			readonly property int column: index % parent.columnCount
			readonly property int row: Math.floor(index / parent.columnCount)

			x: column * parent.pitch - parent.dotRadius
			y: -row * parent.pitch - parent.dotRadius
			width: parent.dotRadius * 2
			height: parent.dotRadius * 2
			radius: parent.dotRadius
			color: column < parent.columns.length && parent.columns[column] > row ? Theme.accent : Theme.emptyDot
		}
	}
}
```

- [ ] **Step 4: Write the tiles**

`qml/TimeTile.qml` (tile rectangle x 15, y 15, 455x220):

```qml
import QtQuick

Tile {
	x: 15
	y: 15
	width: 455
	height: 220
	title: "Time"

	TileText {
		x: 6.2
		baselineY: 156.7
		isBlack: true
		font.pixelSize: Theme.heroSize
		color: Theme.value
		text: app.clock.timeText
	}

	TileText {
		x: 14
		baselineY: 205
		text: app.clock.dateText
	}
}
```

`qml/LightTile.qml` (x 15, y 250, 455x455, green):

```qml
import QtQuick

Tile {
	x: 15
	y: 250
	width: 455
	height: 455
	title: "Light"
	isHighlighted: true

	TileText {
		x: 6.2
		baselineY: 391.7
		isBlack: true
		font.pixelSize: Theme.heroSize
		color: app.light.isOn ? Theme.accent : Theme.label
		text: app.light.isOn ? "ON" : "OFF"
	}

	MouseArea {
		anchors.fill: parent
		onClicked: app.light.Toggle()
	}
}
```

`qml/WeatherTile.qml` (x 485, y 15, 220x220):

```qml
import QtQuick

Tile {
	x: 485
	y: 15
	width: 220
	height: 220
	title: "Weather"

	DotMatrix {
		x: 20.4
		y: 135.3
		columns: app.weather.dotColumns
	}

	TileText {
		x: 12.3
		baselineY: 205
		isBlack: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: app.weather.temperatureText
	}

	TileText {
		id: precipitationText
		x: 208 - width
		baselineY: 205
		isBlack: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: app.weather.precipitationText
	}
}
```

`qml/CalendarTile.qml` (x 485, y 250, 220x220):

```qml
import QtQuick

Tile {
	x: 485
	y: 250
	width: 220
	height: 220
	title: "Calendar"

	TileText {
		x: 12.3
		width: 196
		baselineY: 104.6
		isBlack: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		wrapMode: Text.Wrap
		maximumLineCount: 2
		elide: Text.ElideRight
		lineHeightMode: Text.FixedHeight
		lineHeight: 40
		text: app.calendar.titleText
	}

	TileText {
		x: 14
		baselineY: 205
		text: app.calendar.whenText
	}
}
```

`qml/AlarmTile.qml` (x 485, y 485, 220x220):

```qml
import QtQuick

Tile {
	x: 485
	y: 485
	width: 220
	height: 220
	title: "Alarm"

	TileText {
		x: 12.3
		baselineY: 124.6
		isBlack: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: app.alarm.timeText
	}

	Repeater {
		model: 7

		TileText {
			x: 14 + index * 25.2
			baselineY: 205
			color: app.alarm.activeDays[index] ? Theme.accent : Theme.label
			text: "MTWTFSS"[index]
		}
	}
}
```

`qml/Main.qml`:

```qml
import QtQuick
import QtQuick.Window

Window {
	id: window
	width: 720
	height: 720
	visible: true
	title: "Lightswitch"
	color: "black"
	visibility: app.isFullscreen ? Window.FullScreen : Window.Windowed

	Item {
		anchors.centerIn: parent
		width: 720
		height: 720
		scale: Math.min(window.width, window.height) / 720

		TimeTile {}
		LightTile {}
		WeatherTile {}
		CalendarTile {}
		AlarmTile {}
	}
}
```

- [ ] **Step 5: Build, run tests, start the app**

Run the build and the tests (expected: still `100% tests passed, 0 tests failed out of 8`, and `Lightswitch(.exe)` plus `lightswitch.ini` in `bin/windowsx64/debug/` or `bin/linuxx64/debug/`). Start it:

```powershell
& bin/windowsx64/debug/Lightswitch.exe
```

Expected: a 720x720 window with the five tiles, running clock, `ON`/`OFF` toggling on click, weather values after a moment. The QML loader prints no errors: check the console for `qrc:/...` warnings such as `Theme is not a type` or `module not found` and fix them before continuing. If a font family does not resolve, print `Theme.boldFamily` and `Theme.blackFamily` (expected `Manufaktur` and `Manufaktur Black`).

- [ ] **Step 6: Visual verification against the SVG**

Render `C:\Users\User\Downloads\Lightswitch.svg` (open it in the browser pane, `mcp__Claude_Browser__*`) next to a screenshot of the running app and compare tile positions, text baselines, the dot matrix and colors. Adjust the coordinates in the tile files until they match within 2 px. Record deliberate differences (for example live values instead of the SVG sample text) in the commit message.

- [ ] **Step 7: Commit**

```bash
git add -A
git commit -m "feat: add app controller and Qt Quick user interface"
```

---

### Task 10: Cross-platform verification and cleanup

**Files:**
- Modify: `CLAUDE.md` (only if a documented command changed), `README.md` (short build/run section)

- [ ] **Step 1: Verify all four Windows configurations**

```powershell
foreach ($config in 'debug','release','debug-level-log','release-level-log') {
	cmake --build --preset "windows-x64-$config"
}
ctest --test-dir bin/windowsx64/vs -C Release --output-on-failure
Get-ChildItem bin/windowsx64 -Directory | Select-Object Name
```

Expected: all builds succeed; directories `debug`, `release`, `debug_level_log`, `release_level_log` (plus `vs`) under `bin/windowsx64/`; no other build output folders in the repository root.

- [ ] **Step 2: Verify the live weather fetch**

Start the app with `bin/windowsx64/debug_level_log/Lightswitch.exe` and check that the console shows `lightswitch.weather` debug lines (verbose level), then that the weather tile shows a temperature and dots. If the request fails, confirm the URL independently:

```powershell
Invoke-RestMethod "https://api.open-meteo.com/v1/forecast?latitude=48.1374&longitude=11.5755&current=temperature_2m&hourly=precipitation_probability&forecast_days=2&timezone=auto" | ConvertTo-Json -Depth 4 | Select-Object -First 30
```

Expected keys: `current.time`, `current.temperature_2m`, `hourly.time`, `hourly.precipitation_probability`. Adjust `OpenMeteoParser` and its test only if the real response shape differs.

- [ ] **Step 3: Verify the alarm end to end**

Temporarily copy `config/lightswitch.ini` next to the exe with `time` set to two minutes from now and `days` including today, start with `--config <that file>`, and confirm the light tile switches to `ON` at that minute. Do not commit the temporary file.

- [ ] **Step 4: Verify Linux**

On a Linux machine (or WSL) with Qt 6.4+: `./BuildAndRun.sh debug --install-deps --no-run`, then `./BuildAndRun.sh release --clean --no-run -j 4`, `./BuildAndRun.sh debug -- --fullscreen`. Also check error handling: `./BuildAndRun.sh foo` and `./BuildAndRun.sh --jobs abc` exit with a non-zero code and a clear message. If no Linux machine is available, say so explicitly in the final report instead of claiming it was verified.

- [ ] **Step 5: Quality passes**

Run the `code-simplifier` pass over `src/`, `qml/` and `tests/` (strip dead includes and redundancy, keep behavior), then `code-review` on the branch. The weather service performs network I/O and parses external JSON, so also run the `security-guidance` check on `WeatherService` and `OpenMeteoParser` (HTTPS only, timeout set, malformed input rejected).

- [ ] **Step 6: README and final commit**

Add a short section to `README.md`: what it is, prerequisites (Qt 6.4+, CMake 3.22+, VS 2026 on Windows), the build and run commands from `CLAUDE.md`, and the `lightswitch.ini` options.

```bash
git add -A
git commit -m "docs: add build instructions and finish verification"
```
