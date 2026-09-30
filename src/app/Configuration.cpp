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

// Reads the night mode section; missing or invalid entries keep the values already in the target.
void ReadNightMode(const QSettings& settings, NightModeSettings& nightMode) {
	if (settings.contains(QStringLiteral("nightmode/enabled"))) {
		nightMode.isEnabled = settings.value(QStringLiteral("nightmode/enabled")).toBool();
	}

	const QTime startTime = QTime::fromString(settings.value(QStringLiteral("nightmode/start")).toString().trimmed(), QStringLiteral("HH:mm"));
	if (startTime.isValid()) {
		nightMode.startTime = startTime;
	}

	const QTime endTime = QTime::fromString(settings.value(QStringLiteral("nightmode/end")).toString().trimmed(), QStringLiteral("HH:mm"));
	if (endTime.isValid()) {
		nightMode.endTime = endTime;
	}
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

	ReadNightMode(settings, configuration.m_nightMode);

	return configuration;
}

// Parses a comma separated list of weekday names (first three letters count) into Monday-first flags.
std::array<bool, 7> Configuration::ParseActiveDays(const QString& text) {
	static const QStringList dayPrefixes = {
		QStringLiteral("mon"), QStringLiteral("tue"), QStringLiteral("wed"), QStringLiteral("thu"),
		QStringLiteral("fri"), QStringLiteral("sat"), QStringLiteral("sun")};

	std::array<bool, 7> activeDays = {};
	for (const QString& token : text.split(QLatin1Char(','))) {
		const int dayIndex = static_cast<int>(dayPrefixes.indexOf(token.trimmed().left(3).toLower()));
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

// Returns the night mode schedule.
const NightModeSettings& Configuration::NightMode() const {
	return m_nightMode;
}
