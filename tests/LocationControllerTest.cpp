#include "LocationController.h"

#include <QSignalSpy>
#include <QtTest>

class LocationControllerTest : public QObject {
	Q_OBJECT

private slots:
	void SelectsNextAndPreviousPreset();
	void WrapsAroundAtListEnds();
	void CustomLocationStartsAtListEnds();
	void EmitsLocationChanged();
};

void LocationControllerTest::SelectsNextAndPreviousPreset() {
	LocationController controller(LocationController::Presets().first());

	controller.SelectNext();
	QCOMPARE(controller.Name(), LocationController::Presets().at(1).name);

	controller.SelectPrevious();
	QCOMPARE(controller.Name(), LocationController::Presets().first().name);
}

void LocationControllerTest::WrapsAroundAtListEnds() {
	LocationController controller(LocationController::Presets().first());

	controller.SelectPrevious();
	QCOMPARE(controller.Name(), LocationController::Presets().last().name);

	controller.SelectNext();
	QCOMPARE(controller.Name(), LocationController::Presets().first().name);
}

void LocationControllerTest::CustomLocationStartsAtListEnds() {
	LocationController controller({QStringLiteral("Custom"), 10.0, 20.0});
	controller.SelectNext();
	QCOMPARE(controller.Name(), LocationController::Presets().first().name);

	LocationController otherController({QStringLiteral("Custom"), 10.0, 20.0});
	otherController.SelectPrevious();
	QCOMPARE(otherController.Name(), LocationController::Presets().last().name);
}

void LocationControllerTest::EmitsLocationChanged() {
	LocationController controller(LocationController::Presets().first());
	QSignalSpy spy(&controller, &LocationController::LocationChanged);

	controller.SelectNext();

	QCOMPARE(spy.count(), 1);
	const LocationSettings location = spy.first().first().value<LocationSettings>();
	QCOMPARE(location.name, LocationController::Presets().at(1).name);
	QCOMPARE(location.latitude, LocationController::Presets().at(1).latitude);
}

QTEST_GUILESS_MAIN(LocationControllerTest)
#include "LocationControllerTest.moc"
