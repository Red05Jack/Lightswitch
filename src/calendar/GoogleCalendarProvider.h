#pragma once

#include "GoogleAuthorizer.h"
#include "ICalendarProvider.h"

#include <functional>

#include <QList>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>

class QNetworkReply;

// Loads events, contact birthdays and reminders (Google Tasks) for the next seven days and serves them from a cache.
class GoogleCalendarProvider : public QObject, public ICalendarProvider {
	Q_OBJECT

public:
	static constexpr int lookAheadDays = 7;

	GoogleCalendarProvider(const GoogleCredentials& credentials, const QString& tokenFilePath, QObject* pParent = nullptr);

	QList<CalendarEvent> UpcomingEvents(const QDateTime& from) const override;
	bool NeedsLinking() const override;
	void BeginLinking() override;

	void Start();
	void Refresh();

signals:
	void Changed();
	void AuthorizationUrlReady(const QUrl& url);

private:
	void FetchAll(const QString& accessToken);
	void FetchCalendar(const QString& calendarId, const QString& accessToken, bool isEssential);
	void FetchReminders(const QString& accessToken);
	void Fetch(const QUrl& url, const QString& accessToken, bool isEssential, std::function<void(const QByteArray&)> onSuccess);
	void RebuildEvents();
	void ScheduleRetry();

	GoogleAuthorizer m_authorizer;
	QNetworkAccessManager m_network;
	QTimer m_refreshTimer;
	QTimer m_retryTimer;
	QList<CalendarEvent> m_calendarEvents;
	QList<CalendarEvent> m_birthdayEvents;
	QList<CalendarEvent> m_reminders;
	QList<CalendarEvent> m_events;
};
