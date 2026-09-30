#include "DummyLightController.h"
#include "FakeClock.h"
#include "NightModeController.h"

#include <QSignalSpy>
#include <QtTest>

namespace {
const QDate day(2026, 9, 30);

// Builds a local date time on the test day.
QDateTime At(int hour, int minute, int second = 0) {
	return QDateTime(day, QTime(hour, minute, second));
}

// Night window from 22:00 to 06:00.
NightModeSettings MakeSettings() {
	NightModeSettings settings;
	settings.isEnabled = true;
	settings.startTime = QTime(22, 0);
	settings.endTime = QTime(6, 0);
	return settings;
}
}

class NightModeControllerTest : public QObject {
	Q_OBJECT

private slots:
	void ActivatesAfterFiveIdleMinutesInsideWindow();
	void StaysOffBeforeIdleTimeout();
	void InputRestartsIdlePeriod();
	void StaysOffOutsideWindow();
	void StaysOffWhenDisabled();
	void StaysOffWhileLightIsOn();
	void WindowMayBeWithinOneDay();
	void ManualActivationIgnoresSchedule();
	void WakeDeactivatesAndRestartsIdlePeriod();
	void LightSwitchedOnWakesDisplay();
	void EmitsActiveChangedOnlyOnChange();
};

void NightModeControllerTest::ActivatesAfterFiveIdleMinutesInsideWindow() {
	FakeClock clock(At(23, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());

	clock.SetNow(At(23, 5));
	nightMode.Check();

	QVERIFY(nightMode.IsActive());
}

void NightModeControllerTest::StaysOffBeforeIdleTimeout() {
	FakeClock clock(At(23, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());

	clock.SetNow(At(23, 4, 59));
	nightMode.Check();

	QVERIFY(!nightMode.IsActive());
}

void NightModeControllerTest::InputRestartsIdlePeriod() {
	FakeClock clock(At(23, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());

	clock.SetNow(At(23, 4));
	nightMode.RegisterInput();
	clock.SetNow(At(23, 8));
	nightMode.Check();
	QVERIFY(!nightMode.IsActive());

	clock.SetNow(At(23, 9));
	nightMode.Check();
	QVERIFY(nightMode.IsActive());
}

void NightModeControllerTest::StaysOffOutsideWindow() {
	FakeClock clock(At(12, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());

	clock.SetNow(At(12, 30));
	nightMode.Check();

	QVERIFY(!nightMode.IsActive());
}

void NightModeControllerTest::StaysOffWhenDisabled() {
	FakeClock clock(At(23, 0));
	DummyLightController light;
	NightModeSettings settings = MakeSettings();
	settings.isEnabled = false;
	NightModeController nightMode(clock, light, settings);

	clock.SetNow(At(23, 30));
	nightMode.Check();

	QVERIFY(!nightMode.IsActive());
}

void NightModeControllerTest::StaysOffWhileLightIsOn() {
	FakeClock clock(At(23, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());
	light.SetOn(true);

	clock.SetNow(At(23, 30));
	nightMode.Check();

	QVERIFY(!nightMode.IsActive());
}

void NightModeControllerTest::WindowMayBeWithinOneDay() {
	FakeClock clock(At(12, 0));
	DummyLightController light;
	NightModeSettings settings = MakeSettings();
	settings.startTime = QTime(13, 0);
	settings.endTime = QTime(15, 0);
	NightModeController nightMode(clock, light, settings);

	clock.SetNow(At(13, 10));
	nightMode.Check();
	QVERIFY(nightMode.IsActive());

	nightMode.Wake();
	clock.SetNow(At(15, 30));
	nightMode.Check();
	QVERIFY(!nightMode.IsActive());
}

void NightModeControllerTest::ManualActivationIgnoresSchedule() {
	FakeClock clock(At(12, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());

	nightMode.Activate();

	QVERIFY(nightMode.IsActive());
}

void NightModeControllerTest::WakeDeactivatesAndRestartsIdlePeriod() {
	FakeClock clock(At(23, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());
	clock.SetNow(At(23, 6));
	nightMode.Check();
	QVERIFY(nightMode.IsActive());

	nightMode.Wake();
	QVERIFY(!nightMode.IsActive());

	clock.SetNow(At(23, 10));
	nightMode.Check();
	QVERIFY(!nightMode.IsActive());

	clock.SetNow(At(23, 11));
	nightMode.Check();
	QVERIFY(nightMode.IsActive());
}

void NightModeControllerTest::LightSwitchedOnWakesDisplay() {
	FakeClock clock(At(23, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());
	nightMode.Activate();

	light.SetOn(true);

	QVERIFY(!nightMode.IsActive());
}

void NightModeControllerTest::EmitsActiveChangedOnlyOnChange() {
	FakeClock clock(At(23, 0));
	DummyLightController light;
	NightModeController nightMode(clock, light, MakeSettings());
	QSignalSpy spy(&nightMode, &NightModeController::ActiveChanged);

	nightMode.Activate();
	nightMode.Activate();
	nightMode.Wake();
	nightMode.Wake();

	QCOMPARE(spy.count(), 2);
}

QTEST_GUILESS_MAIN(NightModeControllerTest)
#include "NightModeControllerTest.moc"
