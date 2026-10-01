#pragma once

#include "CalendarEvent.h"

#include <QList>

// Source of calendar events, implemented by the dummy provider and by Google Calendar.
class ICalendarProvider {
public:
	virtual ~ICalendarProvider() = default;

	virtual QList<CalendarEvent> UpcomingEvents(const QDateTime& from) const = 0;

	// Providers that need a one time account link report it here and start it on request.
	virtual bool NeedsLinking() const { return false; }
	virtual void BeginLinking() {}
};
