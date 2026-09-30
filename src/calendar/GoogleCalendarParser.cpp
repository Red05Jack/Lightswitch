#include "GoogleCalendarParser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace {
// Converts one item of the events list; returns false for cancelled or undated entries.
bool ParseEvent(const QJsonObject& item, CalendarEvent& event) {
	if (item.value(QStringLiteral("status")).toString() == QStringLiteral("cancelled")) {
		return false;
	}

	const QJsonObject start = item.value(QStringLiteral("start")).toObject();
	const QString dateTimeText = start.value(QStringLiteral("dateTime")).toString();
	const QString dateText = start.value(QStringLiteral("date")).toString();

	if (!dateTimeText.isEmpty()) {
		event.start = QDateTime::fromString(dateTimeText, Qt::ISODate).toLocalTime();
		event.isAllDay = false;
	} else if (!dateText.isEmpty()) {
		event.start = QDateTime(QDate::fromString(dateText, Qt::ISODate), QTime(0, 0));
		event.isAllDay = true;
	}
	if (!event.start.isValid()) {
		return false;
	}

	event.title = item.value(QStringLiteral("summary")).toString().trimmed();
	if (event.title.isEmpty()) {
		event.title = QStringLiteral("(No title)");
	}
	return true;
}
}

// Reads the "items" array of an events list response; an invalid document yields no events.
QList<CalendarEvent> GoogleCalendarParser::ParseEvents(const QByteArray& json) {
	QList<CalendarEvent> events;
	const QJsonArray items = QJsonDocument::fromJson(json).object().value(QStringLiteral("items")).toArray();
	for (const QJsonValue& item : items) {
		CalendarEvent event;
		if (ParseEvent(item.toObject(), event)) {
			events.append(event);
		}
	}

	std::sort(events.begin(), events.end(), [](const CalendarEvent& left, const CalendarEvent& right) {
		return left.start < right.start;
	});
	return events;
}

// Reads the access token, optional refresh token and lifetime, or the error code of a failed request.
GoogleTokenResponse GoogleCalendarParser::ParseTokenResponse(const QByteArray& json) {
	const QJsonObject object = QJsonDocument::fromJson(json).object();

	GoogleTokenResponse response;
	response.accessToken = object.value(QStringLiteral("access_token")).toString();
	response.refreshToken = object.value(QStringLiteral("refresh_token")).toString();
	response.expiresInSeconds = object.value(QStringLiteral("expires_in")).toInt();
	response.errorCode = object.value(QStringLiteral("error")).toString();
	if (response.accessToken.isEmpty() && response.errorCode.isEmpty()) {
		response.errorCode = QStringLiteral("invalid_response");
	}
	return response;
}

// Keeps events that have not passed yet and start within the look-ahead window; all-day events count by date.
QList<CalendarEvent> GoogleCalendarParser::SelectUpcoming(const QList<CalendarEvent>& events, const QDateTime& from, int lookAheadDays) {
	const QDateTime until = from.addDays(lookAheadDays);

	QList<CalendarEvent> upcoming;
	for (const CalendarEvent& event : events) {
		const bool isNotOver = event.isAllDay ? event.start.date() >= from.date() : event.start >= from;
		const bool isWithinWindow = event.isAllDay ? event.start.date() < until.date() : event.start < until;
		if (isNotOver && isWithinWindow) {
			upcoming.append(event);
		}
	}
	return upcoming;
}
