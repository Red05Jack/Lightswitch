#include "DummyCalendarProvider.h"

#include <QSet>
#include <QtTest>

namespace {
const QDateTime from(QDate(2026, 9, 30), QTime(12, 0));
}

class DummyCalendarProviderTest : public QObject {
	Q_OBJECT

private slots:
	void IsDeterministicForSameSeed();
	void ReturnsSortedEventsNotBeforeFrom();
	void EventsLieWithinDaytime();
	void ProvidesEventsOnEveryUpcomingDay();
};

void DummyCalendarProviderTest::IsDeterministicForSameSeed() {
	const DummyCalendarProvider first(42);
	const DummyCalendarProvider second(42);

	const QList<CalendarEvent> firstEvents = first.UpcomingEvents(from);
	const QList<CalendarEvent> secondEvents = second.UpcomingEvents(from);

	QCOMPARE(firstEvents.size(), secondEvents.size());
	for (int index = 0; index < firstEvents.size(); ++index) {
		QCOMPARE(firstEvents.at(index).title, secondEvents.at(index).title);
		QCOMPARE(firstEvents.at(index).start, secondEvents.at(index).start);
	}
}

void DummyCalendarProviderTest::ReturnsSortedEventsNotBeforeFrom() {
	const DummyCalendarProvider provider(7);

	const QList<CalendarEvent> events = provider.UpcomingEvents(from);

	QVERIFY(!events.isEmpty());
	for (int index = 0; index < events.size(); ++index) {
		QVERIFY(events.at(index).start >= from);
		if (index > 0) {
			QVERIFY(events.at(index - 1).start <= events.at(index).start);
		}
	}
}

void DummyCalendarProviderTest::EventsLieWithinDaytime() {
	const DummyCalendarProvider provider(99);

	for (const CalendarEvent& event : provider.UpcomingEvents(from)) {
		QVERIFY(!event.title.isEmpty());
		QVERIFY(event.start.time().hour() >= 8);
		QVERIFY(event.start.time().hour() <= 20);
	}
}

void DummyCalendarProviderTest::ProvidesEventsOnEveryUpcomingDay() {
	const DummyCalendarProvider provider(3);
	const QDateTime startOfDay(QDate(2026, 9, 30), QTime(0, 0));

	QSet<QDate> days;
	for (const CalendarEvent& event : provider.UpcomingEvents(startOfDay)) {
		days.insert(event.start.date());
	}

	QCOMPARE(days.size(), 7);
}

QTEST_GUILESS_MAIN(DummyCalendarProviderTest)
#include "DummyCalendarProviderTest.moc"
