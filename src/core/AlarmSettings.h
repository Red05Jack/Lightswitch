#pragma once

#include <QTime>

#include <array>

// Alarm of a single weekday.
struct AlarmDay {
	bool isActive = false;
	QTime time = QTime(6, 45);
};

// Global alarm switch and one alarm per weekday (Monday first).
struct AlarmSettings {
	static constexpr int dayCount = 7;

	AlarmSettings() {
		for (int dayIndex = 0; dayIndex < 5; ++dayIndex) {
			days.at(static_cast<size_t>(dayIndex)).isActive = true;
		}
	}

	bool isEnabled = true;
	std::array<AlarmDay, dayCount> days;
};
