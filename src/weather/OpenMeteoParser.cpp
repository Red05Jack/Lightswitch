#include "OpenMeteoParser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

#include <algorithm>

namespace {
constexpr int forecastHourCount = 24;
constexpr int hourPrefixLength = 13; // Length of "yyyy-MM-ddTHH".
}

// Extracts the current temperature and the next 24 hourly rain probabilities, or nothing if the data is unusable.
std::optional<WeatherForecast> OpenMeteoParser::Parse(const QByteArray& json) {
	const QJsonDocument document = QJsonDocument::fromJson(json);
	if (!document.isObject()) {
		return std::nullopt;
	}

	const QJsonObject root = document.object();
	const QJsonObject current = root.value(QStringLiteral("current")).toObject();
	const QJsonObject hourly = root.value(QStringLiteral("hourly")).toObject();

	const QJsonValue temperature = current.value(QStringLiteral("temperature_2m"));
	const QString currentTime = current.value(QStringLiteral("time")).toString();
	const QJsonArray times = hourly.value(QStringLiteral("time")).toArray();
	const QJsonArray probabilities = hourly.value(QStringLiteral("precipitation_probability")).toArray();

	if (!temperature.isDouble() || currentTime.size() < hourPrefixLength || times.isEmpty() || times.size() != probabilities.size()) {
		return std::nullopt;
	}

	const QString currentHour = currentTime.left(hourPrefixLength);
	int startIndex = -1;
	for (int index = 0; index < times.size(); ++index) {
		if (times.at(index).toString().startsWith(currentHour)) {
			startIndex = index;
			break;
		}
	}
	if (startIndex < 0) {
		return std::nullopt;
	}

	WeatherForecast forecast;
	forecast.temperatureCelsius = temperature.toDouble();
	const int endIndex = std::min(startIndex + forecastHourCount, static_cast<int>(probabilities.size()));
	for (int index = startIndex; index < endIndex; ++index) {
		forecast.hourlyPrecipitationPercent.append(probabilities.at(index).toInt(0));
	}
	return forecast;
}

// Builds the forecast request URL for the given location in degrees.
QUrl OpenMeteoParser::BuildRequestUrl(double latitude, double longitude) {
	QUrlQuery query;
	query.addQueryItem(QStringLiteral("latitude"), QString::number(latitude, 'f', 4));
	query.addQueryItem(QStringLiteral("longitude"), QString::number(longitude, 'f', 4));
	query.addQueryItem(QStringLiteral("current"), QStringLiteral("temperature_2m"));
	query.addQueryItem(QStringLiteral("hourly"), QStringLiteral("precipitation_probability"));
	query.addQueryItem(QStringLiteral("forecast_days"), QStringLiteral("2"));
	query.addQueryItem(QStringLiteral("timezone"), QStringLiteral("auto"));

	QUrl url(QStringLiteral("https://api.open-meteo.com/v1/forecast"));
	url.setQuery(query);
	return url;
}
