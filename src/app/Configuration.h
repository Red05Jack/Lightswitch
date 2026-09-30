#pragma once

#include "AlarmSettings.h"
#include "NightModeSettings.h"

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
	const NightModeSettings& NightMode() const;

private:
	double m_latitude = 48.1374;
	double m_longitude = 11.5755;
	AlarmSettings m_alarm;
	NightModeSettings m_nightMode;
};
