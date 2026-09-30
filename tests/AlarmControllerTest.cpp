#include "AlarmController.h"
#include "DummyLightController.h"
#include "FakeClock.h"

#include <QSignalSpy>
#include <QtTest>

namespace {
const QDate wednesday(2026, 9, 30);
const QDate thursday(2026, 10, 1);
const QDate saturday(2026, 10, 3);

// Returns Monday to Friday at 06:45 and Saturday and Sunday off.
AlarmSettings MakeWeekdaySettings() {
	return AlarmSettings();
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
	void UsesTheTimeOfEachWeekday();
	void NeverTriggersWhenSwitchedOff();
	void ShowsNextAlarmToday();
	void ShowsNextAlarmOnFollowingDay();
	void ShowsOffWithoutActiveDays();
	void ShowsOffWhenSwitchedOff();
	void EditsDayTimeAndNotifies();
	void IgnoresInvalidDayIndex();
	void ExposesActiveDaysAndTimeTexts();
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

void AlarmControllerTest::UsesTheTimeOfEachWeekday() {
	AlarmSettings settings = MakeWeekdaySettings();
	settings.days.at(2).time = QTime(7, 30);
	FakeClock clock(At(wednesday, 6, 45));
	DummyLightController light;
	AlarmController alarm(clock, light, settings);

	alarm.Check();
	QVERIFY(!light.IsOn());

	clock.SetNow(At(wednesday, 7, 30));
	alarm.Check();
	QVERIFY(light.IsOn());
}

void AlarmControllerTest::NeverTriggersWhenSwitchedOff() {
	AlarmSettings settings = MakeWeekdaySettings();
	settings.isEnabled = false;
	FakeClock clock(At(wednesday, 6, 45));
	DummyLightController light;
	AlarmController alarm(clock, light, settings);

	alarm.Check();

	QVERIFY(!light.IsOn());
}

void AlarmControllerTest::ShowsNextAlarmToday() {
	AlarmSettings settings = MakeWeekdaySettings();
	settings.days.at(2).time = QTime(7, 30);
	FakeClock clock(At(wednesday, 6, 0));
	DummyLightController light;

	const AlarmController alarm(clock, light, settings);

	QCOMPARE(alarm.NextAlarmText(), QStringLiteral("07:30"));
}

void AlarmControllerTest::ShowsNextAlarmOnFollowingDay() {
	AlarmSettings settings = MakeWeekdaySettings();
	settings.days.at(3).time = QTime(8, 15);
	FakeClock clock(At(wednesday, 12, 0));
	DummyLightController light;
	AlarmController alarm(clock, light, settings);
	QCOMPARE(alarm.NextAlarmText(), QStringLiteral("08:15"));

	clock.SetNow(At(saturday, 12, 0));
	alarm.Check();
	QCOMPARE(alarm.NextAlarmText(), QStringLiteral("06:45"));
}

void AlarmControllerTest::ShowsOffWithoutActiveDays() {
	AlarmSettings settings;
	for (AlarmDay& day : settings.days) {
		day.isActive = false;
	}
	FakeClock clock(At(wednesday, 12, 0));
	DummyLightController light;

	const AlarmController alarm(clock, light, settings);

	QCOMPARE(alarm.NextAlarmText(), QStringLiteral("OFF"));
}

void AlarmControllerTest::ShowsOffWhenSwitchedOff() {
	FakeClock clock(At(wednesday, 5, 0));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QCOMPARE(alarm.NextAlarmText(), QStringLiteral("06:45"));

	alarm.SetEnabled(false);

	QCOMPARE(alarm.NextAlarmText(), QStringLiteral("OFF"));
	QVERIFY(!alarm.IsEnabled());
}

void AlarmControllerTest::EditsDayTimeAndNotifies() {
	FakeClock clock(At(wednesday, 5, 0));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QSignalSpy changedSpy(&alarm, &AlarmController::Changed);
	QSignalSpy settingsSpy(&alarm, &AlarmController::SettingsChanged);

	alarm.ShiftDayTime(2, 60);
	alarm.ShiftDayTime(2, -5);
	alarm.SetDayActive(5, true);

	QCOMPARE(alarm.TimeTexts().at(2).toString(), QStringLiteral("07:40"));
	QVERIFY(alarm.ActiveDays().at(5).toBool());
	QCOMPARE(alarm.NextAlarmText(), QStringLiteral("07:40"));
	QCOMPARE(changedSpy.count(), 3);
	QCOMPARE(settingsSpy.count(), 3);
}

void AlarmControllerTest::IgnoresInvalidDayIndex() {
	FakeClock clock(At(wednesday, 5, 0));
	DummyLightController light;
	AlarmController alarm(clock, light, MakeWeekdaySettings());
	QSignalSpy spy(&alarm, &AlarmController::SettingsChanged);

	alarm.ShiftDayTime(-1, 5);
	alarm.ShiftDayTime(7, 5);
	alarm.SetDayActive(9, true);

	QCOMPARE(spy.count(), 0);
}

void AlarmControllerTest::ExposesActiveDaysAndTimeTexts() {
	FakeClock clock(At(wednesday, 0, 0));
	DummyLightController light;
	const AlarmController alarm(clock, light, MakeWeekdaySettings());

	const QVariantList days = alarm.ActiveDays();
	QCOMPARE(days.size(), 7);
	QVERIFY(days.at(0).toBool());
	QVERIFY(days.at(4).toBool());
	QVERIFY(!days.at(5).toBool());
	QVERIFY(!days.at(6).toBool());
	QCOMPARE(alarm.TimeTexts().size(), 7);
	QCOMPARE(alarm.TimeTexts().at(0).toString(), QStringLiteral("06:45"));
}

QTEST_GUILESS_MAIN(AlarmControllerTest)
#include "AlarmControllerTest.moc"
