#include "GoogleAuthorizer.h"
#include "GoogleCalendarParser.h"

#include <QTimeZone>
#include <QtTest>

namespace {
const QByteArray eventsJson = R"({
	"items": [
		{ "status": "confirmed", "summary": "Dentist", "start": { "dateTime": "2026-10-02T14:30:00+02:00" } },
		{ "status": "confirmed", "summary": "Birthday", "start": { "date": "2026-10-01" } },
		{ "status": "cancelled", "summary": "Cancelled", "start": { "dateTime": "2026-10-01T10:00:00+02:00" } },
		{ "status": "confirmed", "start": { "dateTime": "2026-10-03T09:00:00Z" } },
		{ "status": "confirmed", "summary": "No start" }
	]
})";

CalendarEvent MakeEvent(const QString& title, const QDateTime& start, bool isAllDay = false) {
	CalendarEvent event;
	event.title = title;
	event.start = start;
	event.isAllDay = isAllDay;
	return event;
}
}

class GoogleCalendarParserTest : public QObject {
	Q_OBJECT

private slots:
	void ParsesTimedAndAllDayEvents();
	void SkipsCancelledAndUndatedEvents();
	void SortsEventsByStart();
	void InvalidJsonYieldsNoEvents();
	void ParsesTokenResponse();
	void ParsesTokenError();
	void RejectsEmptyTokenResponse();
	void SelectsEventsInsideWindow();
	void KeepsAllDayEventOfToday();
	void ComputesPkceChallengeOfRfcExample();
};

void GoogleCalendarParserTest::ParsesTimedAndAllDayEvents() {
	const QList<CalendarEvent> events = GoogleCalendarParser::ParseEvents(eventsJson);

	QCOMPARE(events.size(), 3);
	QCOMPARE(events.at(0).title, QStringLiteral("Birthday"));
	QVERIFY(events.at(0).isAllDay);
	QCOMPARE(events.at(0).start, QDateTime(QDate(2026, 10, 1), QTime(0, 0)));
	QCOMPARE(events.at(1).title, QStringLiteral("Dentist"));
	QVERIFY(!events.at(1).isAllDay);
	QCOMPARE(events.at(1).start.toUTC(), QDateTime(QDate(2026, 10, 2), QTime(12, 30), QTimeZone::UTC));
}

void GoogleCalendarParserTest::SkipsCancelledAndUndatedEvents() {
	const QList<CalendarEvent> events = GoogleCalendarParser::ParseEvents(eventsJson);

	for (const CalendarEvent& event : events) {
		QVERIFY(event.title != QStringLiteral("Cancelled"));
		QVERIFY(event.title != QStringLiteral("No start"));
	}
}

void GoogleCalendarParserTest::SortsEventsByStart() {
	const QList<CalendarEvent> events = GoogleCalendarParser::ParseEvents(eventsJson);

	QCOMPARE(events.last().title, QStringLiteral("(No title)"));
}

void GoogleCalendarParserTest::InvalidJsonYieldsNoEvents() {
	QVERIFY(GoogleCalendarParser::ParseEvents("not json").isEmpty());
	QVERIFY(GoogleCalendarParser::ParseEvents("{}").isEmpty());
}

void GoogleCalendarParserTest::ParsesTokenResponse() {
	const GoogleTokenResponse response = GoogleCalendarParser::ParseTokenResponse(
		R"({"access_token":"abc","expires_in":3599,"refresh_token":"xyz","token_type":"Bearer"})");

	QVERIFY(response.errorCode.isEmpty());
	QCOMPARE(response.accessToken, QStringLiteral("abc"));
	QCOMPARE(response.refreshToken, QStringLiteral("xyz"));
	QCOMPARE(response.expiresInSeconds, 3599);
}

void GoogleCalendarParserTest::ParsesTokenError() {
	const GoogleTokenResponse response = GoogleCalendarParser::ParseTokenResponse(R"({"error":"invalid_grant"})");

	QCOMPARE(response.errorCode, QStringLiteral("invalid_grant"));
	QVERIFY(response.accessToken.isEmpty());
}

void GoogleCalendarParserTest::RejectsEmptyTokenResponse() {
	QVERIFY(!GoogleCalendarParser::ParseTokenResponse("garbage").errorCode.isEmpty());
}

void GoogleCalendarParserTest::SelectsEventsInsideWindow() {
	const QDateTime now(QDate(2026, 10, 1), QTime(12, 0));
	const QList<CalendarEvent> events = {
		MakeEvent(QStringLiteral("Past"), QDateTime(QDate(2026, 10, 1), QTime(9, 0))),
		MakeEvent(QStringLiteral("Soon"), QDateTime(QDate(2026, 10, 1), QTime(18, 0))),
		MakeEvent(QStringLiteral("Edge"), QDateTime(QDate(2026, 10, 8), QTime(11, 0))),
		MakeEvent(QStringLiteral("Too late"), QDateTime(QDate(2026, 10, 8), QTime(12, 0)))};

	const QList<CalendarEvent> upcoming = GoogleCalendarParser::SelectUpcoming(events, now, 7);

	QCOMPARE(upcoming.size(), 2);
	QCOMPARE(upcoming.at(0).title, QStringLiteral("Soon"));
	QCOMPARE(upcoming.at(1).title, QStringLiteral("Edge"));
}

void GoogleCalendarParserTest::KeepsAllDayEventOfToday() {
	const QDateTime now(QDate(2026, 10, 1), QTime(12, 0));
	const QList<CalendarEvent> events = {
		MakeEvent(QStringLiteral("Yesterday"), QDateTime(QDate(2026, 9, 30), QTime(0, 0)), true),
		MakeEvent(QStringLiteral("Today"), QDateTime(QDate(2026, 10, 1), QTime(0, 0)), true),
		MakeEvent(QStringLiteral("Later"), QDateTime(QDate(2026, 10, 8), QTime(0, 0)), true)};

	const QList<CalendarEvent> upcoming = GoogleCalendarParser::SelectUpcoming(events, now, 7);

	QCOMPARE(upcoming.size(), 1);
	QCOMPARE(upcoming.at(0).title, QStringLiteral("Today"));
}

void GoogleCalendarParserTest::ComputesPkceChallengeOfRfcExample() {
	QCOMPARE(GoogleAuthorizer::CodeChallenge(QStringLiteral("dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk")),
		QStringLiteral("E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM"));
}

QTEST_GUILESS_MAIN(GoogleCalendarParserTest)
#include "GoogleCalendarParserTest.moc"
