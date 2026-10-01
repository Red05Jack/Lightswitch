#pragma once

#include "WeatherForecast.h"

#include <QByteArray>
#include <QUrl>

#include <optional>

// Builds Open-Meteo requests and converts their JSON responses into WeatherForecast objects.
class OpenMeteoParser {
public:
	static std::optional<WeatherForecast> Parse(const QByteArray& json);
	static QUrl BuildRequestUrl(double latitude, double longitude);
};
