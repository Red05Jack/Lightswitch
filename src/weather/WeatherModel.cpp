#include "WeatherModel.h"

#include <algorithm>

namespace {
constexpr int maximumPercent = 100;
constexpr int percentPerDot = maximumPercent / WeatherModel::maximumDotCount;

// Returns the placeholder shown while no forecast is available.
QString Placeholder() {
	return QStringLiteral("--");
}
}

// Groups the hourly probabilities into 2 hour columns and converts each maximum into 0..5 dots.
QList<int> WeatherModel::ComputeDotColumns(const QList<int>& hourlyPercent) {
	QList<int> columns;
	const int availableHours = static_cast<int>(hourlyPercent.size());

	for (int column = 0; column < columnCount; ++column) {
		int columnMaximum = 0;
		for (int hour = 0; hour < hoursPerColumn; ++hour) {
			const int index = column * hoursPerColumn + hour;
			if (index < availableHours) {
				columnMaximum = std::max(columnMaximum, std::clamp(hourlyPercent.at(index), 0, maximumPercent));
			}
		}
		columns.append((columnMaximum + percentPerDot - 1) / percentPerDot);
	}
	return columns;
}

WeatherModel::WeatherModel(QObject* pParent)
	: QObject(pParent)
	, m_temperatureText(Placeholder())
	, m_precipitationText(Placeholder())
	, m_dotColumns(ComputeDotColumns(QList<int>())) {
}

// Returns whether a forecast has been received yet.
bool WeatherModel::HasData() const {
	return m_hasData;
}

// Returns the rounded temperature with degree sign, or a placeholder.
QString WeatherModel::TemperatureText() const {
	return m_temperatureText;
}

// Returns the rain probability of the current hour, or a placeholder.
QString WeatherModel::PrecipitationText() const {
	return m_precipitationText;
}

// Returns the dot count of each of the 12 columns for the QML dot matrix.
QVariantList WeatherModel::DotColumns() const {
	QVariantList columns;
	for (const int dotCount : m_dotColumns) {
		columns.append(dotCount);
	}
	return columns;
}

// Replaces the displayed data with the given forecast.
void WeatherModel::SetForecast(const WeatherForecast& forecast) {
	m_hasData = true;
	m_temperatureText = QString::number(qRound(forecast.temperatureCelsius)) + QChar(0x00B0);
	m_precipitationText = forecast.hourlyPrecipitationPercent.isEmpty()
		? Placeholder()
		: QString::number(forecast.hourlyPrecipitationPercent.first()) + QLatin1Char('%');
	m_dotColumns = ComputeDotColumns(forecast.hourlyPrecipitationPercent);
	emit Changed();
}
