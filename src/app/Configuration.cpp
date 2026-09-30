#include "Configuration.h"

#include <QSettings>
#include <QStringList>

namespace {
constexpr double maximumLatitude = 90.0;
constexpr double maximumLongitude = 180.0;
constexpr int coordinateDecimals = 4;
const QString timeFormat = QStringLiteral("HH:mm");

// Lower case weekday prefixes, Monday first; they are the keys of the alarm times and the tokens of the day list.
const QStringList& DayKeys() {
	static const QStringList dayKeys = {
		QStringLiteral("mon"), QStringLiteral("tue"), QStringLiteral("wed"), QStringLiteral("thu"),
		QStringLiteral("fri"), QStringLiteral("sat"), QStringLiteral("sun")};
	return dayKeys;
}

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

// Reads the alarm section: "time" is the default time for all days, "mon".."sun" override it per day.
void ReadAlarm(const QSettings& settings, AlarmSettings& alarm) {
	if (settings.contains(QStringLiteral("alarm/enabled"))) {
		alarm.isEnabled = settings.value(QStringLiteral("alarm/enabled")).toBool();
	}

	const QTime defaultTime = QTime::fromString(settings.value(QStringLiteral("alarm/time")).toString().trimmed(), timeFormat);
	for (int dayIndex = 0; dayIndex < AlarmSettings::dayCount; ++dayIndex) {
		AlarmDay& day = alarm.days.at(static_cast<size_t>(dayIndex));
		if (defaultTime.isValid()) {
			day.time = defaultTime;
		}

		const QString dayKey = QStringLiteral("alarm/") + DayKeys().at(dayIndex);
		const QTime dayTime = QTime::fromString(settings.value(dayKey).toString().trimmed(), timeFormat);
		if (dayTime.isValid()) {
			day.time = dayTime;
		}
	}

	if (settings.contains(QStringLiteral("alarm/days"))) {
		const QString daysText = settings.value(QStringLiteral("alarm/days")).toStringList().join(QLatin1Char(','));
		const std::array<bool, 7> activeDays = Configuration::ParseActiveDays(daysText);
		for (int dayIndex = 0; dayIndex < AlarmSettings::dayCount; ++dayIndex) {
			alarm.days.at(static_cast<size_t>(dayIndex)).isActive = activeDays.at(static_cast<size_t>(dayIndex));
		}
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

	ReadAlarm(settings, configuration.m_alarm);
	ReadNightMode(settings, configuration.m_nightMode);

	return configuration;
}

// Parses a comma separated list of weekday names (first three letters count) into Monday-first flags.
std::array<bool, 7> Configuration::ParseActiveDays(const QString& text) {
	std::array<bool, 7> activeDays = {};
	for (const QString& token : text.split(QLatin1Char(','))) {
		const int dayIndex = static_cast<int>(DayKeys().indexOf(token.trimmed().left(3).toLower()));
		if (dayIndex >= 0) {
			activeDays.at(static_cast<size_t>(dayIndex)) = true;
		}
	}
	return activeDays;
}

// Formats Monday-first flags as a comma separated list of weekday keys such as "mon,tue".
QString Configuration::FormatActiveDays(const std::array<bool, 7>& activeDays) {
	QStringList activeKeys;
	for (int dayIndex = 0; dayIndex < AlarmSettings::dayCount; ++dayIndex) {
		if (activeDays.at(static_cast<size_t>(dayIndex))) {
			activeKeys.append(DayKeys().at(dayIndex));
		}
	}
	return activeKeys.join(QLatin1Char(','));
}

// Writes the location, alarm and night mode sections and keeps all other entries of the file; returns false on failure.
bool Configuration::Save(const QString& filePath) const {
	QSettings settings(filePath, QSettings::IniFormat);

	settings.setValue(QStringLiteral("location/name"), m_location.name);
	settings.setValue(QStringLiteral("location/latitude"), QString::number(m_location.latitude, 'f', coordinateDecimals));
	settings.setValue(QStringLiteral("location/longitude"), QString::number(m_location.longitude, 'f', coordinateDecimals));

	std::array<bool, 7> activeDays = {};
	settings.remove(QStringLiteral("alarm/time"));
	settings.setValue(QStringLiteral("alarm/enabled"), m_alarm.isEnabled);
	for (int dayIndex = 0; dayIndex < AlarmSettings::dayCount; ++dayIndex) {
		const AlarmDay& day = m_alarm.days.at(static_cast<size_t>(dayIndex));
		activeDays.at(static_cast<size_t>(dayIndex)) = day.isActive;
		settings.setValue(QStringLiteral("alarm/") + DayKeys().at(dayIndex), day.time.toString(timeFormat));
	}
	settings.setValue(QStringLiteral("alarm/days"), FormatActiveDays(activeDays));

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

// Replaces the alarm settings.
void Configuration::SetAlarm(const AlarmSettings& alarm) {
	m_alarm = alarm;
}

// Replaces the night mode schedule.
void Configuration::SetNightMode(const NightModeSettings& nightMode) {
	m_nightMode = nightMode;
}
