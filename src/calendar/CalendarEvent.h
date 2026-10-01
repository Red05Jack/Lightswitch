#pragma once

#include <QDateTime>
#include <QString>

// Distinguishes real appointments from reminders and birthdays, which have a lower priority on the home screen.
enum class CalendarEventKind {
	Event,
	Reminder,
	Birthday
};

// A single calendar entry with its title and start time; all-day entries only have a date.
struct CalendarEvent {
	QString title;
	QDateTime start;
	bool isAllDay = false;
	CalendarEventKind kind = CalendarEventKind::Event;
};
