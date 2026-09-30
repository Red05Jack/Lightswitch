#include "CalendarModel.h"
#include "FakeClock.h"

#include <QSignalSpy>
#include <QtTest>

namespace {
// Provider returning a fixed list of events.
class StubCalendarProvider : public ICalendarProvider {
public:
	explicit StubCalendarProvider(const QList<CalendarEvent>& events) : m_events(events) {}

	QList<CalendarEvent> UpcomingEvents(const QDateTime&) const override { return m_events; }

	bool NeedsLinking() const override { return m_needsLinking; }
	void BeginLinking() override { m_linkingStarted = true; }

	void SetEvents(const QList<CalendarEvent>& events) { m_events = events; }
	void SetNeedsLinking(bool needsLinking) { m_needsLinking = needsLinking; }
	bool WasLinkingStarted() const { return m_linkingStarted; }

private:
	QList<CalendarEvent> m_events;
	bool m_needsLinking = false;
	bool m_linkingStarted = false;
};
}

class CalendarModelTest : public QObject {
	Q_OBJECT

private slots:
	void ShowsNextEvent();
	void ShowsPlaceholderWithoutEvents();
	void RefreshUpdatesAndEmitsChanged();
	void ShowsDateOnlyForAllDayEvents();
	void AsksToLinkAccount();
};

void CalendarModelTest::ShowsNextEvent() {
	FakeClock clock(QDateTime(QDate(2026, 9, 30), QTime(12, 0)));
	StubCalendarProvider provider({{QStringLiteral("Dentist"), QDateTime(QDate(2026, 10, 1), QTime(13, 30))}});

	const CalendarModel model(provider, clock);

	QCOMPARE(model.TitleText(), QStringLiteral("Dentist"));
	QCOMPARE(model.WhenText(), QStringLiteral("01.10.2026") + QChar(0x2022) + QStringLiteral("13:30"));
}

void CalendarModelTest::ShowsPlaceholderWithoutEvents() {
	FakeClock clock(QDateTime(QDate(2026, 9, 30), QTime(12, 0)));
	StubCalendarProvider provider({});

	const CalendarModel model(provider, clock);

	QCOMPARE(model.TitleText(), QStringLiteral("No events"));
	QVERIFY(model.WhenText().isEmpty());
}

void CalendarModelTest::RefreshUpdatesAndEmitsChanged() {
	FakeClock clock(QDateTime(QDate(2026, 9, 30), QTime(12, 0)));
	StubCalendarProvider provider({});
	CalendarModel model(provider, clock);
	QSignalSpy spy(&model, &CalendarModel::Changed);

	model.Refresh();
	QCOMPARE(spy.count(), 0);

	provider.SetEvents({{QStringLiteral("Gym"), QDateTime(QDate(2026, 9, 30), QTime(18, 0))}});
	model.Refresh();
	QCOMPARE(spy.count(), 1);
	QCOMPARE(model.TitleText(), QStringLiteral("Gym"));
}

void CalendarModelTest::ShowsDateOnlyForAllDayEvents() {
	FakeClock clock(QDateTime(QDate(2026, 9, 30), QTime(12, 0)));
	CalendarEvent event;
	event.title = QStringLiteral("Holiday");
	event.start = QDateTime(QDate(2026, 10, 3), QTime(0, 0));
	event.isAllDay = true;
	StubCalendarProvider provider({event});

	const CalendarModel model(provider, clock);

	QCOMPARE(model.WhenText(), QStringLiteral("03.10.2026"));
}

void CalendarModelTest::AsksToLinkAccount() {
	FakeClock clock(QDateTime(QDate(2026, 9, 30), QTime(12, 0)));
	StubCalendarProvider provider({});
	provider.SetNeedsLinking(true);
	CalendarModel model(provider, clock);
	QSignalSpy spy(&model, &CalendarModel::Changed);

	QVERIFY(model.NeedsLinking());
	QCOMPARE(model.TitleText(), QStringLiteral("Link Google"));

	model.BeginLinking();
	QVERIFY(provider.WasLinkingStarted());

	provider.SetNeedsLinking(false);
	model.Refresh();
	QCOMPARE(model.TitleText(), QStringLiteral("No events"));
	QCOMPARE(spy.count(), 1);
}

QTEST_GUILESS_MAIN(CalendarModelTest)
#include "CalendarModelTest.moc"
