#include "DummyCalendarProvider.h"

#include <QRandomGenerator>
#include <QStringList>

#include <algorithm>

namespace {
constexpr int lookAheadDays = 7;
constexpr int firstEventHour = 8;
constexpr int lastEventHour = 20;
constexpr int minutesPerQuarterHour = 15;
constexpr int quarterHoursPerHour = 4;
constexpr int maximumEventsPerDay = 2;
}

DummyCalendarProvider::DummyCalendarProvider(quint32 seed)
	: m_seed(seed) {
}

// Returns the generated events of the next seven days that start at or after the given time.
QList<CalendarEvent> DummyCalendarProvider::UpcomingEvents(const QDateTime& from) const {
	QList<CalendarEvent> events;
	for (int dayOffset = 0; dayOffset < lookAheadDays; ++dayOffset) {
		for (const CalendarEvent& event : EventsForDate(from.date().addDays(dayOffset))) {
			if (event.start >= from) {
				events.append(event);
			}
		}
	}
	return events;
}

// Generates one or two sorted events for the date, derived only from the seed and the date.
QList<CalendarEvent> DummyCalendarProvider::EventsForDate(const QDate& date) const {
	static const QStringList titles = {
		QStringLiteral("Dinner with Dad"), QStringLiteral("Team meeting"), QStringLiteral("Dentist"),
		QStringLiteral("Call with Anna"), QStringLiteral("Grocery shopping"), QStringLiteral("Gym")};

	QRandomGenerator generator(m_seed + static_cast<quint32>(date.toJulianDay()));
	const int eventCount = generator.bounded(1, maximumEventsPerDay + 1);

	QList<CalendarEvent> events;
	for (int index = 0; index < eventCount; ++index) {
		CalendarEvent event;
		event.title = titles.at(generator.bounded(static_cast<int>(titles.size())));
		const int hour = generator.bounded(firstEventHour, lastEventHour + 1);
		const int minute = generator.bounded(quarterHoursPerHour) * minutesPerQuarterHour;
		event.start = QDateTime(date, QTime(hour, minute));
		events.append(event);
	}

	std::sort(events.begin(), events.end(), [](const CalendarEvent& left, const CalendarEvent& right) {
		return left.start < right.start;
	});
	return events;
}
