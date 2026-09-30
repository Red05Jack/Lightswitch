#include "GoogleCalendarProvider.h"

#include "GoogleCalendarParser.h"

#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

Q_LOGGING_CATEGORY(lcGoogleCalendar, "lightswitch.calendar.google")

namespace {
const QString eventsEndpoint = QStringLiteral("https://www.googleapis.com/calendar/v3/calendars/primary/events");
constexpr int refreshIntervalMilliseconds = 10 * 60 * 1000;
constexpr int retryIntervalMilliseconds = 60 * 1000;
constexpr int requestTimeoutMilliseconds = 15 * 1000;
constexpr int maximumEventCount = 50;
}

GoogleCalendarProvider::GoogleCalendarProvider(const GoogleCredentials& credentials, const QString& tokenFilePath, QObject* pParent)
	: QObject(pParent)
	, m_authorizer(credentials, tokenFilePath) {
	m_refreshTimer.setInterval(refreshIntervalMilliseconds);
	connect(&m_refreshTimer, &QTimer::timeout, this, &GoogleCalendarProvider::Refresh);

	// Retrying soon matters when the device starts before the network is ready.
	m_retryTimer.setSingleShot(true);
	m_retryTimer.setInterval(retryIntervalMilliseconds);
	connect(&m_retryTimer, &QTimer::timeout, this, &GoogleCalendarProvider::Refresh);

	connect(&m_authorizer, &GoogleAuthorizer::AccessTokenReady, this, &GoogleCalendarProvider::FetchEvents);
	connect(&m_authorizer, &GoogleAuthorizer::RequestFailed, this, &GoogleCalendarProvider::ScheduleRetry);
	connect(&m_authorizer, &GoogleAuthorizer::AuthorizationUrlReady, this, &GoogleCalendarProvider::AuthorizationUrlReady);
	connect(&m_authorizer, &GoogleAuthorizer::LinkedChanged, this, [this]() {
		m_events.clear();
		emit Changed();
		Refresh();
	});
}

// Returns the cached events of the next seven days that have not passed yet.
QList<CalendarEvent> GoogleCalendarProvider::UpcomingEvents(const QDateTime& from) const {
	return GoogleCalendarParser::SelectUpcoming(m_events, from, lookAheadDays);
}

// Returns whether the Google account still has to be linked.
bool GoogleCalendarProvider::NeedsLinking() const {
	return !m_authorizer.IsLinked();
}

// Starts the browser based account linking.
void GoogleCalendarProvider::BeginLinking() {
	m_authorizer.BeginLinking();
}

// Loads the events immediately and then keeps refreshing them.
void GoogleCalendarProvider::Start() {
	Refresh();
	m_refreshTimer.start();
}

// Requests an access token; the events are fetched as soon as it is available.
void GoogleCalendarProvider::Refresh() {
	m_retryTimer.stop();
	if (NeedsLinking()) {
		emit Changed();
		return;
	}
	m_authorizer.RequestAccessToken();
}

// Requests the events from now until the end of the look-ahead window.
void GoogleCalendarProvider::FetchEvents(const QString& accessToken) {
	const QDateTime now = QDateTime::currentDateTimeUtc();

	QUrlQuery query;
	query.addQueryItem(QStringLiteral("timeMin"), now.toString(Qt::ISODate));
	query.addQueryItem(QStringLiteral("timeMax"), now.addDays(lookAheadDays).toString(Qt::ISODate));
	query.addQueryItem(QStringLiteral("singleEvents"), QStringLiteral("true"));
	query.addQueryItem(QStringLiteral("orderBy"), QStringLiteral("startTime"));
	query.addQueryItem(QStringLiteral("maxResults"), QString::number(maximumEventCount));

	QUrl url{eventsEndpoint};
	url.setQuery(query);
	QNetworkRequest request(url);
	request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
	request.setTransferTimeout(requestTimeoutMilliseconds);

	QNetworkReply* pReply = m_network.get(request);
	connect(pReply, &QNetworkReply::finished, this, [this, pReply]() { HandleEventsReply(pReply); });
}

// Replaces the cached events with the parsed reply; failures are logged and the old events stay visible.
void GoogleCalendarProvider::HandleEventsReply(QNetworkReply* pReply) {
	pReply->deleteLater();

	if (pReply->error() != QNetworkReply::NoError) {
		qCWarning(lcGoogleCalendar) << "Events request failed:" << pReply->errorString();
		ScheduleRetry();
		return;
	}

	m_events = GoogleCalendarParser::ParseEvents(pReply->readAll());
	qCDebug(lcGoogleCalendar) << "Received" << m_events.size() << "events";
	emit Changed();
}

// Tries again after a short pause.
void GoogleCalendarProvider::ScheduleRetry() {
	m_retryTimer.start();
}
