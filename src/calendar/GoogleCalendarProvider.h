#pragma once

#include "GoogleAuthorizer.h"
#include "ICalendarProvider.h"

#include <QList>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>

class QNetworkReply;

// Loads the events of the primary Google calendar for the next seven days and serves them from a cache.
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
	void FetchEvents(const QString& accessToken);
	void HandleEventsReply(QNetworkReply* pReply);
	void ScheduleRetry();

	GoogleAuthorizer m_authorizer;
	QNetworkAccessManager m_network;
	QTimer m_refreshTimer;
	QTimer m_retryTimer;
	QList<CalendarEvent> m_events;
};
