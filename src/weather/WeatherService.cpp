#include "WeatherService.h"

#include "OpenMeteoParser.h"

#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>

Q_LOGGING_CATEGORY(lcWeather, "lightswitch.weather")

namespace {
constexpr int refreshIntervalMilliseconds = 30 * 60 * 1000;
constexpr int requestTimeoutMilliseconds = 15 * 1000;
}

WeatherService::WeatherService(double latitude, double longitude, QObject* pParent)
	: QObject(pParent)
	, m_requestUrl(OpenMeteoParser::BuildRequestUrl(latitude, longitude)) {
	m_refreshTimer.setInterval(refreshIntervalMilliseconds);
	connect(&m_refreshTimer, &QTimer::timeout, this, &WeatherService::Refresh);
}

// Fetches the forecast immediately and then keeps refreshing it.
void WeatherService::Start() {
	Refresh();
	m_refreshTimer.start();
}

// Requests a fresh forecast; failures are logged and the previous data stays visible.
void WeatherService::Refresh() {
	QNetworkRequest request(m_requestUrl);
	request.setTransferTimeout(requestTimeoutMilliseconds);

	QNetworkReply* pReply = m_network.get(request);
	connect(pReply, &QNetworkReply::finished, this, [this, pReply]() { HandleReply(pReply); });
	qCDebug(lcWeather) << "Requesting forecast" << m_requestUrl;
}

// Converts a finished reply into a forecast or logs why that was not possible.
void WeatherService::HandleReply(QNetworkReply* pReply) {
	pReply->deleteLater();

	if (pReply->error() != QNetworkReply::NoError) {
		qCWarning(lcWeather) << "Forecast request failed:" << pReply->errorString();
		return;
	}

	const std::optional<WeatherForecast> forecast = OpenMeteoParser::Parse(pReply->readAll());
	if (!forecast.has_value()) {
		qCWarning(lcWeather) << "Forecast response could not be parsed.";
		return;
	}

	qCDebug(lcWeather) << "Forecast received, temperature" << forecast->temperatureCelsius;
	emit ForecastReady(*forecast);
}
