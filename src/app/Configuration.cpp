#include "Configuration.h"

#include <QSettings>
#include <QStringList>

namespace {
constexpr double maximumLatitude = 90.0;
constexpr double maximumLongitude = 180.0;
constexpr int coordinateDecimals = 4;
const QString timeFormat = QStringLiteral("HH:mm");

// Reads a coordinate within +-limit, or returns the fallback when it is missing or invalid.
double ReadCoordinate(const QSettings& settings, const QString& key, double fallback, double limit) {
	bool isNumber = false;
	const double value = settings.value(key).toString().toDouble(&isNumber);
	if (!isNumber || value < -limit || value > limit) {
		return fallback;
	}
	return value;
}

// Reads the location section; coordinates that differ from the default and have no name are shown as "Custom".
void ReadLocation(const QSettings& settings, LocationSettings& location) {
	const LocationSettings defaultLocation = location;
	location.latitude = ReadCoordinate(settings, QStringLiteral("location/latitude"), location.latitude, maximumLatitude);
	location.longitude = ReadCoordinate(settings, QStringLiteral("location/longitude"), location.longitude, maximumLongitude);

	const QString name = settings.value(QStringLiteral("location/name")).toString().trimmed();
	if (!name.isEmpty()) {
		location.name = name;
	} else if (location.latitude != defaultLocation.latitude || location.longitude != defaultLocation.longitude) {
		location.name = QStringLiteral("Custom");
	}
}

// Reads the night mode section; missing or invalid entries keep the values already in the target.
void ReadNightMode(const QSettings& settings, NightModeSettings& nightMode) {
	if (settings.contains(QStringLiteral("nightmode/enabled"))) {
		nightMode.isEnabled = settings.value(QStringLiteral("nightmode/enabled")).toBool();
	}

	const QTime startTime = QTime::fromString(settings.value(QStringLiteral("nightmode/start")).toString().trimmed(), timeFormat);
	if (startTime.isValid()) {
		nightMode.startTime = startTime;
	}

	const QTime endTime = QTime::fromString(settings.value(QStringLiteral("nightmode/end")).toString().trimmed(), timeFormat);
	if (endTime.isValid()) {
		nightMode.endTime = endTime;
	}
}
}

// Loads the configuration file and keeps the defaults for everything that is missing or invalid.
Configuration Configuration::Load(const QString& filePath) {
	const QSettings settings(filePath, QSettings::IniFormat);
	Configuration configuration;

	ReadLocation(settings, configuration.m_location);

	const QTime alarmTime = QTime::fromString(settings.value(QStringLiteral("alarm/time")).toString().trimmed(), timeFormat);
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

// Writes the location and night mode sections and keeps all other entries of the file; returns false on failure.
bool Configuration::Save(const QString& filePath) const {
	QSettings settings(filePath, QSettings::IniFormat);

	settings.setValue(QStringLiteral("location/name"), m_location.name);
	settings.setValue(QStringLiteral("location/latitude"), QString::number(m_location.latitude, 'f', coordinateDecimals));
	settings.setValue(QStringLiteral("location/longitude"), QString::number(m_location.longitude, 'f', coordinateDecimals));

	settings.setValue(QStringLiteral("nightmode/enabled"), m_nightMode.isEnabled);
	settings.setValue(QStringLiteral("nightmode/start"), m_nightMode.startTime.toString(timeFormat));
	settings.setValue(QStringLiteral("nightmode/end"), m_nightMode.endTime.toString(timeFormat));

	settings.sync();
	return settings.status() == QSettings::NoError;
}

// Returns the weather location.
const LocationSettings& Configuration::Location() const {
	return m_location;
}

// Returns the alarm time and active weekdays.
const AlarmSettings& Configuration::Alarm() const {
	return m_alarm;
}

// Returns the night mode schedule.
const NightModeSettings& Configuration::NightMode() const {
	return m_nightMode;
}

// Replaces the weather location.
void Configuration::SetLocation(const LocationSettings& location) {
	m_location = location;
}

// Replaces the night mode schedule.
void Configuration::SetNightMode(const NightModeSettings& nightMode) {
	m_nightMode = nightMode;
}
