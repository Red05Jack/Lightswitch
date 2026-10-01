#pragma once

#include "WeatherForecast.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>

// Turns a weather forecast into the texts and the 12x5 rain dot matrix shown on the weather tile.
class WeatherModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool hasData READ HasData NOTIFY Changed)
	Q_PROPERTY(QString temperatureText READ TemperatureText NOTIFY Changed)
	Q_PROPERTY(QString precipitationText READ PrecipitationText NOTIFY Changed)
	Q_PROPERTY(QVariantList dotColumns READ DotColumns NOTIFY Changed)

public:
	static constexpr int columnCount = 12;
	static constexpr int hoursPerColumn = 2;
	static constexpr int maximumDotCount = 5;

	static QList<int> ComputeDotColumns(const QList<int>& hourlyPercent);

	explicit WeatherModel(QObject* pParent = nullptr);

	bool HasData() const;
	QString TemperatureText() const;
	QString PrecipitationText() const;
	QVariantList DotColumns() const;

	void SetForecast(const WeatherForecast& forecast);

signals:
	void Changed();

private:
	bool m_hasData = false;
	QString m_temperatureText;
	QString m_precipitationText;
	QList<int> m_dotColumns;
};
