#include "DummyLightController.h"

#include <QSignalSpy>
#include <QtTest>

class DummyLightControllerTest : public QObject {
	Q_OBJECT

private slots:
	void StartsOff();
	void SetOnEmitsOnlyOnChange();
	void ToggleFlipsState();
};

void DummyLightControllerTest::StartsOff() {
	DummyLightController light;

	QVERIFY(!light.IsOn());
}

void DummyLightControllerTest::SetOnEmitsOnlyOnChange() {
	DummyLightController light;
	QSignalSpy spy(&light, &ILightController::StateChanged);

	light.SetOn(true);
	light.SetOn(true);

	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.at(0).at(0).toBool(), true);
	QVERIFY(light.IsOn());
}

void DummyLightControllerTest::ToggleFlipsState() {
	DummyLightController light;

	light.Toggle();
	QVERIFY(light.IsOn());
	light.Toggle();
	QVERIFY(!light.IsOn());
}

QTEST_GUILESS_MAIN(DummyLightControllerTest)
#include "DummyLightControllerTest.moc"
