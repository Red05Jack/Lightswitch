#pragma once

#include "AlarmSettings.h"
#include "LocationSettings.h"
#include "NightModeSettings.h"

#include <QString>

#include <array>

// Application settings loaded from and saved to an INI file, with defaults for missing or invalid entries.
class Configuration {
public:
	static Configuration Load(const QString& filePath);
	static std::array<bool, 7> ParseActiveDays(const QString& text);

	bool Save(const QString& filePath) const;

	const LocationSettings& Location() const;
	const AlarmSettings& Alarm() const;
	const NightModeSettings& NightMode() const;

	void SetLocation(const LocationSettings& location);
	void SetNightMode(const NightModeSettings& nightMode);

private:
	LocationSettings m_location;
	AlarmSettings m_alarm;
	NightModeSettings m_nightMode;
};
