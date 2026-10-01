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
