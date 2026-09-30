#pragma once

#include "CalendarEvent.h"

#include <QByteArray>
#include <QList>

// Result of a token request to Google; errorCode is empty on success.
struct GoogleTokenResponse {
	QString accessToken;
	QString refreshToken;
	int expiresInSeconds = 0;
	QString errorCode;
};

// Parses the JSON documents of the Google Calendar and OAuth endpoints.
class GoogleCalendarParser {
public:
	static QList<CalendarEvent> ParseEvents(const QByteArray& json);
	static GoogleTokenResponse ParseTokenResponse(const QByteArray& json);
	static QList<CalendarEvent> SelectUpcoming(const QList<CalendarEvent>& events, const QDateTime& from, int lookAheadDays);
};
