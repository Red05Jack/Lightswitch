#include "WeatherModel.h"

#include <QSignalSpy>
#include <QtTest>

class WeatherModelTest : public QObject {
	Q_OBJECT

private slots:
	void EmptyInputGivesZeroDots();
	void DotCountIsCeilOfPercentDividedByTwenty();
	void ColumnTakesMaximumOfItsTwoHours();
	void ValuesAreClamped();
	void ShortListLeavesRemainingColumnsEmpty();
	void InitialStateShowsPlaceholders();
	void SetForecastUpdatesTexts();
};

void WeatherModelTest::EmptyInputGivesZeroDots() {
	const QList<int> columns = WeatherModel::ComputeDotColumns({});

	QCOMPARE(columns, (QList<int>(12, 0)));
}

void WeatherModelTest::DotCountIsCeilOfPercentDividedByTwenty() {
	QCOMPARE(WeatherModel::ComputeDotColumns({0}).at(0), 0);
	QCOMPARE(WeatherModel::ComputeDotColumns({1}).at(0), 1);
	QCOMPARE(WeatherModel::ComputeDotColumns({20}).at(0), 1);
	QCOMPARE(WeatherModel::ComputeDotColumns({21}).at(0), 2);
	QCOMPARE(WeatherModel::ComputeDotColumns({80}).at(0), 4);
	QCOMPARE(WeatherModel::ComputeDotColumns({100}).at(0), 5);
}

void WeatherModelTest::ColumnTakesMaximumOfItsTwoHours() {
	const QList<int> columns = WeatherModel::ComputeDotColumns({0, 40, 100, 10});

	QCOMPARE(columns.at(0), 2);
	QCOMPARE(columns.at(1), 5);
}

void WeatherModelTest::ValuesAreClamped() {
	const QList<int> columns = WeatherModel::ComputeDotColumns({150, -20});

	QCOMPARE(columns.at(0), 5);
}

void WeatherModelTest::ShortListLeavesRemainingColumnsEmpty() {
	const QList<int> columns = WeatherModel::ComputeDotColumns({100, 100, 100});

	QCOMPARE(columns.size(), 12);
	QCOMPARE(columns.at(0), 5);
	QCOMPARE(columns.at(1), 5);
	QCOMPARE(columns.at(2), 0);
	QCOMPARE(columns.at(11), 0);
}

void WeatherModelTest::InitialStateShowsPlaceholders() {
	WeatherModel model;

	QVERIFY(!model.HasData());
	QCOMPARE(model.TemperatureText(), QStringLiteral("--"));
	QCOMPARE(model.PrecipitationText(), QStringLiteral("--"));
	QCOMPARE(model.DotColumns().size(), 12);
}

void WeatherModelTest::SetForecastUpdatesTexts() {
	WeatherModel model;
	QSignalSpy spy(&model, &WeatherModel::Changed);
	WeatherForecast forecast;
	forecast.temperatureCelsius = 11.4;
	forecast.hourlyPrecipitationPercent = {27, 0, 60};

	model.SetForecast(forecast);

	QVERIFY(model.HasData());
	QCOMPARE(model.TemperatureText(), QString::number(11) + QChar(0x00B0));
	QCOMPARE(model.PrecipitationText(), QStringLiteral("27%"));
	QCOMPARE(model.DotColumns().at(0).toInt(), 2);
	QCOMPARE(model.DotColumns().at(1).toInt(), 3);
	QCOMPARE(spy.count(), 1);
}

QTEST_GUILESS_MAIN(WeatherModelTest)
#include "WeatherModelTest.moc"
