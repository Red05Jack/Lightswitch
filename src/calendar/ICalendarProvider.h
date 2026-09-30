#pragma once

#include "CalendarEvent.h"

#include <QList>

// Source of calendar events, implemented by the dummy provider now and by Google Calendar later.
class ICalendarProvider {
public:
	virtual ~ICalendarProvider() = default;

	virtual QList<CalendarEvent> UpcomingEvents(const QDateTime& from) const = 0;
};
