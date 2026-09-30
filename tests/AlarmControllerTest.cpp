#include "AlarmController.h"
#include "DummyLightController.h"
#include "FakeClock.h"

#include <QSignalSpy>
#include <QtTest>

namespace {
const QDate wednesday(2026, 9, 30);
const QDate thursday(2026, 10, 1);
const QDate saturday(2026, 10, 3);

// Returns Monday to Friday at 06:45.
AlarmSettings MakeWeekdaySettings() {
	AlarmSettings settings;
	settings.time = QTime(6, 45);
	settings.activeDays = {true, true, true, true, true, false, false};
	return settings;
}

// Builds a local date time for the given day and time of day.
QDateTime At(const QDate& date, int hour, int minute, int second = 0) {
	return QDateTime(date, QTime(hour, minute, second));
}
}

class AlarmControllerTest : public QObject {
	Q_OBJECT

private slots:
	void TriggersOnActiveDayAtAlarmMinute();
	void IgnoresInactiveDays();
	void IgnoresOtherMinutes();
	void TriggersOnlyOncePerDay();
	void TriggersAgainOnNextActiveDay();
	void NeverSwitchesLightOff();
	void ExposesTimeTextAndActiveDays();
};

void AlarmControllerTest::TriggersOnActiveDayAtAlarmMinute() {
	FakeClock clock(At(wednesday, 6, 45, 10));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QSignalSpy spy(&alarm, &AlarmController::Triggered);

	alarm.Check();

	QVERIFY(light.IsOn());
	QCOMPARE(spy.count(), 1);
}

void AlarmControllerTest::IgnoresInactiveDays() {
	FakeClock clock(At(saturday, 6, 45));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());

	alarm.Check();

	QVERIFY(!light.IsOn());
}

void AlarmControllerTest::IgnoresOtherMinutes() {
	FakeClock clock(At(wednesday, 6, 44, 59));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());

	alarm.Check();
	QVERIFY(!light.IsOn());

	clock.SetNow(At(wednesday, 6, 46));
	alarm.Check();
	QVERIFY(!light.IsOn());
}

void AlarmControllerTest::TriggersOnlyOncePerDay() {
	FakeClock clock(At(wednesday, 6, 45, 0));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QSignalSpy spy(&alarm, &AlarmController::Triggered);

	alarm.Check();
	light.SetOn(false);
	clock.SetNow(At(wednesday, 6, 45, 30));
	alarm.Check();

	QVERIFY(!light.IsOn());
	QCOMPARE(spy.count(), 1);
}

void AlarmControllerTest::TriggersAgainOnNextActiveDay() {
	FakeClock clock(At(wednesday, 6, 45));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QSignalSpy spy(&alarm, &AlarmController::Triggered);

	alarm.Check();
	light.SetOn(false);
	clock.SetNow(At(thursday, 6, 45));
	alarm.Check();

	QVERIFY(light.IsOn());
	QCOMPARE(spy.count(), 2);
}

void AlarmControllerTest::NeverSwitchesLightOff() {
	FakeClock clock(At(wednesday, 6, 45));
	DummyLightController light;
	light.SetOn(true);
	AlarmController alarm(clock, light, MakeWeekdaySettings());

	alarm.Check();

	QVERIFY(light.IsOn());
}

void AlarmControllerTest::ExposesTimeTextAndActiveDays() {
	FakeClock clock(At(wednesday, 0, 0));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());

	QCOMPARE(alarm.TimeText(), QStringLiteral("06:45"));
	const QVariantList days = alarm.ActiveDays();
	QCOMPARE(days.size(), 7);
	QVERIFY(days.at(0).toBool());
	QVERIFY(days.at(4).toBool());
	QVERIFY(!days.at(5).toBool());
	QVERIFY(!days.at(6).toBool());
}

QTEST_GUILESS_MAIN(AlarmControllerTest)
#include "AlarmControllerTest.moc"
