#pragma once

#include <QDateTime>
#include <QString>

// A single calendar entry with its title and start time.
struct CalendarEvent {
	QString title;
	QDateTime start;
};
