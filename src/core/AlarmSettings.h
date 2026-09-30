#pragma once

#include <QTime>

#include <array>

// Alarm time and the weekdays (Monday first) on which the alarm is active.
struct AlarmSettings {
	QTime time = QTime(6, 45);
	std::array<bool, 7> activeDays = {true, true, true, true, true, false, false};
};
