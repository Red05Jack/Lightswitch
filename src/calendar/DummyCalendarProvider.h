#pragma once

#include "ICalendarProvider.h"

#include <QtGlobal>

// Generates reproducible random events for the next days, used until a real calendar is connected.
class DummyCalendarProvider : public ICalendarProvider {
public:
	explicit DummyCalendarProvider(quint32 seed);

	QList<CalendarEvent> UpcomingEvents(const QDateTime& from) const override;

private:
	QList<CalendarEvent> EventsForDate(const QDate& date) const;

	quint32 m_seed;
};
