#pragma once

#include "WeatherForecast.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QUrl>

class QNetworkReply;

// Fetches the local forecast from Open-Meteo periodically and publishes it as WeatherForecast.
class WeatherService : public QObject {
	Q_OBJECT

public:
	WeatherService(double latitude, double longitude, QObject* pParent = nullptr);
	explicit WeatherService(const QUrl& requestUrl, QObject* pParent = nullptr);

	void SetRetryInterval(int milliseconds);
	void SetLocation(double latitude, double longitude);
	void Start();
	void Refresh();

signals:
	void ForecastReady(const WeatherForecast& forecast);

private:
	void HandleReply(QNetworkReply* pReply);

	QNetworkAccessManager m_network;
	QPointer<QNetworkReply> m_pendingReply;
	QTimer m_refreshTimer;
	QTimer m_retryTimer;
	QUrl m_requestUrl;
};
