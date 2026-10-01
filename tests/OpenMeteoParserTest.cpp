#include "OpenMeteoParser.h"

#include <QUrlQuery>
#include <QtTest>

namespace {
const QByteArray sampleResponse = R"({
	"current": { "time": "2026-09-30T01:20", "temperature_2m": 11.2 },
	"hourly": {
		"time": ["2026-09-30T00:00", "2026-09-30T01:00", "2026-09-30T02:00", "2026-09-30T03:00"],
		"precipitation_probability": [10, null, 30, 40]
	}
})";
}

class OpenMeteoParserTest : public QObject {
	Q_OBJECT

private slots:
	void ParsesTemperatureAndStartsAtCurrentHour();
	void TreatsNullProbabilityAsZero();
	void KeepsAtMostTwentyFourHours();
	void RejectsInvalidJson();
	void RejectsMissingCurrentBlock();
	void RejectsCurrentHourNotInList();
	void BuildsRequestUrl();
};

void OpenMeteoParserTest::ParsesTemperatureAndStartsAtCurrentHour() {
	const std::optional<WeatherForecast> forecast = OpenMeteoParser::Parse(sampleResponse);

	QVERIFY(forecast.has_value());
	QCOMPARE(forecast->temperatureCelsius, 11.2);
	QCOMPARE(forecast->hourlyPrecipitationPercent, (QList<int>{0, 30, 40}));
}

void OpenMeteoParserTest::TreatsNullProbabilityAsZero() {
	const std::optional<WeatherForecast> forecast = OpenMeteoParser::Parse(sampleResponse);

	QVERIFY(forecast.has_value());
	QCOMPARE(forecast->hourlyPrecipitationPercent.first(), 0);
}

void OpenMeteoParserTest::KeepsAtMostTwentyFourHours() {
	const QDateTime start(QDate(2026, 9, 30), QTime(0, 0));
	QStringList times;
	QStringList probabilities;
	for (int hour = 0; hour < 48; ++hour) {
		times << QLatin1Char('"') + start.addSecs(hour * 3600).toString(QStringLiteral("yyyy-MM-ddTHH:mm")) + QLatin1Char('"');
		probabilities << QString::number(hour);
	}
	const QByteArray json = QStringLiteral(
		R"({"current":{"time":"2026-09-30T00:15","temperature_2m":5.0},)"
		R"("hourly":{"time":[%1],"precipitation_probability":[%2]}})")
		.arg(times.join(QLatin1Char(',')), probabilities.join(QLatin1Char(','))).toUtf8();

	const std::optional<WeatherForecast> forecast = OpenMeteoParser::Parse(json);

	QVERIFY(forecast.has_value());
	QCOMPARE(forecast->hourlyPrecipitationPercent.size(), 24);
	QCOMPARE(forecast->hourlyPrecipitationPercent.last(), 23);
}

void OpenMeteoParserTest::RejectsInvalidJson() {
	QVERIFY(!OpenMeteoParser::Parse("not json").has_value());
	QVERIFY(!OpenMeteoParser::Parse("").has_value());
	QVERIFY(!OpenMeteoParser::Parse("[]").has_value());
}

void OpenMeteoParserTest::RejectsMissingCurrentBlock() {
	const QByteArray json = R"({"hourly":{"time":["2026-09-30T00:00"],"precipitation_probability":[10]}})";

	QVERIFY(!OpenMeteoParser::Parse(json).has_value());
}

void OpenMeteoParserTest::RejectsCurrentHourNotInList() {
	const QByteArray json = R"({
		"current": { "time": "2026-09-30T23:10", "temperature_2m": 11.2 },
		"hourly": { "time": ["2026-09-30T00:00"], "precipitation_probability": [10] }
	})";

	QVERIFY(!OpenMeteoParser::Parse(json).has_value());
}

void OpenMeteoParserTest::BuildsRequestUrl() {
	const QUrl url = OpenMeteoParser::BuildRequestUrl(48.1374, 11.5755);
	const QUrlQuery query(url);

	QCOMPARE(url.host(), QStringLiteral("api.open-meteo.com"));
	QCOMPARE(query.queryItemValue(QStringLiteral("latitude")), QStringLiteral("48.1374"));
	QCOMPARE(query.queryItemValue(QStringLiteral("longitude")), QStringLiteral("11.5755"));
	QCOMPARE(query.queryItemValue(QStringLiteral("current")), QStringLiteral("temperature_2m"));
	QCOMPARE(query.queryItemValue(QStringLiteral("hourly")), QStringLiteral("precipitation_probability"));
	QCOMPARE(query.queryItemValue(QStringLiteral("timezone")), QStringLiteral("auto"));
}

QTEST_GUILESS_MAIN(OpenMeteoParserTest)
#include "OpenMeteoParserTest.moc"
