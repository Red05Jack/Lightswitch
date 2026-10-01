#pragma once

#include <QTime>

// Night mode schedule: the screen may go dark between the start and end time (the window may span midnight).
struct NightModeSettings {
	bool isEnabled = true;
	QTime startTime = QTime(22, 0);
	QTime endTime = QTime(6, 0);
};
