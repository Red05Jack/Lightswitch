#pragma once

#include "WeatherForecast.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>

class QNetworkReply;

// Fetches the local forecast from Open-Meteo periodically and publishes it as WeatherForecast.
class WeatherService : public QObject {
	Q_OBJECT

public:
	WeatherService(double latitude, double longitude, QObject* pParent = nullptr);

	void Start();
	void Refresh();

signals:
	void ForecastReady(const WeatherForecast& forecast);

private:
	void HandleReply(QNetworkReply* pReply);

	QNetworkAccessManager m_network;
	QTimer m_refreshTimer;
	QUrl m_requestUrl;
};
