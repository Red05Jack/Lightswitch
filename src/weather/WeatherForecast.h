#pragma once

#include <QList>

// Weather data needed by the weather tile: the current temperature and the hourly rain probability.
struct WeatherForecast {
	double temperatureCelsius = 0.0;
	QList<int> hourlyPrecipitationPercent;
};
