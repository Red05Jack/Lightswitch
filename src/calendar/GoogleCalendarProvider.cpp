#include "GoogleCalendarProvider.h"

#include "GoogleCalendarParser.h"

#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#include <algorithm>

Q_LOGGING_CATEGORY(lcGoogleCalendar, "lightswitch.calendar.google")

namespace {
const QString calendarsEndpoint = QStringLiteral("https://www.googleapis.com/calendar/v3/calendars/");
const QString tasksEndpoint = QStringLiteral("https://tasks.googleapis.com/tasks/v1/lists/@default/tasks");
const QString primaryCalendarId = QStringLiteral("primary");
// Birthdays of the Google contacts live in this hidden calendar.
const QString birthdayCalendarId = QStringLiteral("addressbook#contacts@group.v.calendar.google.com");
constexpr int refreshIntervalMilliseconds = 5 * 60 * 1000;
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

	connect(&m_authorizer, &GoogleAuthorizer::AccessTokenReady, this, &GoogleCalendarProvider::FetchAll);
	connect(&m_authorizer, &GoogleAuthorizer::RequestFailed, this, &GoogleCalendarProvider::ScheduleRetry);
	connect(&m_authorizer, &GoogleAuthorizer::AuthorizationUrlReady, this, &GoogleCalendarProvider::AuthorizationUrlReady);
	connect(&m_authorizer, &GoogleAuthorizer::LinkedChanged, this, [this]() {
		m_calendarEvents.clear();
		m_birthdayEvents.clear();
		m_reminders.clear();
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

// Requests events, birthdays and reminders; only the primary calendar triggers a retry when it fails.
void GoogleCalendarProvider::FetchAll(const QString& accessToken) {
	FetchCalendar(primaryCalendarId, accessToken, true);
	FetchCalendar(birthdayCalendarId, accessToken, false);
	FetchReminders(accessToken);
}

// Requests the events of one calendar from now until the end of the look-ahead window.
void GoogleCalendarProvider::FetchCalendar(const QString& calendarId, const QString& accessToken, bool isEssential) {
	const QDateTime now = QDateTime::currentDateTimeUtc();

	QUrlQuery query;
	query.addQueryItem(QStringLiteral("timeMin"), now.toString(Qt::ISODate));
	query.addQueryItem(QStringLiteral("timeMax"), now.addDays(lookAheadDays).toString(Qt::ISODate));
	query.addQueryItem(QStringLiteral("singleEvents"), QStringLiteral("true"));
	query.addQueryItem(QStringLiteral("orderBy"), QStringLiteral("startTime"));
	query.addQueryItem(QStringLiteral("maxResults"), QString::number(maximumEventCount));

	QUrl url{calendarsEndpoint + QString::fromLatin1(QUrl::toPercentEncoding(calendarId)) + QStringLiteral("/events")};
	url.setQuery(query);

	const bool isBirthdayCalendar = calendarId == birthdayCalendarId;
	Fetch(url, accessToken, isEssential, [this, isBirthdayCalendar](const QByteArray& body) {
		if (isBirthdayCalendar) {
			m_birthdayEvents = GoogleCalendarParser::ParseEvents(body, CalendarEventKind::Birthday);
		} else {
			m_calendarEvents = GoogleCalendarParser::ParseEvents(body);
		}
		RebuildEvents();
	});
}

// Requests the open tasks of the default list that are due within the look-ahead window; Calendar reminders are stored there.
void GoogleCalendarProvider::FetchReminders(const QString& accessToken) {
	const QDate today = QDate::currentDate();

	QUrlQuery query;
	query.addQueryItem(QStringLiteral("showCompleted"), QStringLiteral("false"));
	query.addQueryItem(QStringLiteral("dueMin"), today.toString(Qt::ISODate) + QStringLiteral("T00:00:00Z"));
	query.addQueryItem(QStringLiteral("dueMax"), today.addDays(lookAheadDays).toString(Qt::ISODate) + QStringLiteral("T00:00:00Z"));
	query.addQueryItem(QStringLiteral("maxResults"), QString::number(maximumEventCount));

	QUrl url{tasksEndpoint};
	url.setQuery(query);
	Fetch(url, accessToken, false, [this](const QByteArray& body) {
		m_reminders = GoogleCalendarParser::ParseReminders(body);
		RebuildEvents();
	});
}

// Sends an authorized GET request; a failure is logged and only retried when the data is essential.
void GoogleCalendarProvider::Fetch(const QUrl& url, const QString& accessToken, bool isEssential, std::function<void(const QByteArray&)> onSuccess) {
	QNetworkRequest request(url);
	request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
	request.setTransferTimeout(requestTimeoutMilliseconds);

	QNetworkReply* pReply = m_network.get(request);
	connect(pReply, &QNetworkReply::finished, this, [this, pReply, isEssential, onSuccess = std::move(onSuccess)]() {
		pReply->deleteLater();

		if (pReply->error() != QNetworkReply::NoError) {
			const int statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
			qCWarning(lcGoogleCalendar) << "Request failed:" << pReply->url().path() << "status" << statusCode << pReply->readAll().left(400);
			if (isEssential) {
				ScheduleRetry();
			}
			return;
		}

		onSuccess(pReply->readAll());
	});
}

// Merges all sources into one list sorted by start time.
void GoogleCalendarProvider::RebuildEvents() {
	m_events = m_calendarEvents + m_birthdayEvents + m_reminders;
	std::stable_sort(m_events.begin(), m_events.end(), [](const CalendarEvent& left, const CalendarEvent& right) {
		return left.start < right.start;
	});
	qCDebug(lcGoogleCalendar) << "Cached" << m_events.size() << "entries";
	emit Changed();
}

// Tries again after a short pause.
void GoogleCalendarProvider::ScheduleRetry() {
	m_retryTimer.start();
}
