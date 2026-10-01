#include "WeatherService.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>

#include <optional>

namespace {
constexpr int failingConnectionCount = 3;
constexpr int retryIntervalMilliseconds = 100;
constexpr int waitTimeoutMilliseconds = 5000;

const QByteArray responseBody = R"({
	"current": { "time": "2026-09-30T01:20", "temperature_2m": 11.2 },
	"hourly": {
		"time": ["2026-09-30T00:00", "2026-09-30T01:00"],
		"precipitation_probability": [10, 30]
	}
})";
}

class WeatherServiceTest : public QObject {
	Q_OBJECT

private slots:
	void RetriesAfterFailedRequest();
};

void WeatherServiceTest::RetriesAfterFailedRequest() {
	QTcpServer server;
	QVERIFY(server.listen(QHostAddress::LocalHost));

	int connectionCount = 0;
	connect(&server, &QTcpServer::newConnection, &server, [&server, &connectionCount]() {
		QTcpSocket* pSocket = server.nextPendingConnection();
		++connectionCount;
		if (connectionCount <= failingConnectionCount) {
			pSocket->abort();
			pSocket->deleteLater();
			return;
		}

		connect(pSocket, &QTcpSocket::readyRead, pSocket, [pSocket]() {
			pSocket->readAll();
			pSocket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length: "
				+ QByteArray::number(responseBody.size()) + "\r\n\r\n" + responseBody);
			pSocket->disconnectFromHost();
		});
	});

	WeatherService service(QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(server.serverPort())));
	service.SetRetryInterval(retryIntervalMilliseconds);
	std::optional<WeatherForecast> received;
	connect(&service, &WeatherService::ForecastReady, &service, [&received](const WeatherForecast& forecast) {
		received = forecast;
	});

	service.Start();

	QTRY_VERIFY_WITH_TIMEOUT(received.has_value(), waitTimeoutMilliseconds);
	QCOMPARE(received->temperatureCelsius, 11.2);
}

QTEST_GUILESS_MAIN(WeatherServiceTest)
#include "WeatherServiceTest.moc"
