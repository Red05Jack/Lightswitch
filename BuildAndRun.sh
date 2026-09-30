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
