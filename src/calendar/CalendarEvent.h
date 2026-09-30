#pragma once

#include <QDateTime>
#include <QString>

// A single calendar entry with its title and start time; all-day events only have a date.
struct CalendarEvent {
	QString title;
	QDateTime start;
	bool isAllDay = false;
};
