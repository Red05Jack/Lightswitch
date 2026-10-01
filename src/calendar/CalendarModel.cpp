#include "CalendarModel.h"

#include <QVariantMap>

#include <algorithm>

namespace {
constexpr int refreshIntervalMilliseconds = 60 * 1000;

QString FormatWhen(const CalendarEvent& event) {
	QString whenText = event.start.toString(QStringLiteral("dd.MM.yyyy"));
	if (!event.isAllDay) {
		whenText += QChar(0x2022) + event.start.toString(QStringLiteral("HH:mm"));
	}
	return whenText;
}

QString KindName(CalendarEventKind kind) {
	switch (kind) {
	case CalendarEventKind::Reminder:
		return QStringLiteral("reminder");
	case CalendarEventKind::Birthday:
		return QStringLiteral("birthday");
	case CalendarEventKind::Event:
		break;
	}
	return QStringLiteral("event");
}

QVariantList ToEntries(const QList<CalendarEvent>& events) {
	QVariantList entries;
	for (const CalendarEvent& event : events) {
		entries.append(QVariantMap{
			{QStringLiteral("title"), event.title},
			{QStringLiteral("whenText"), FormatWhen(event)},
			{QStringLiteral("kind"), KindName(event.kind)}});
	}
	return entries;
}

// Real events take precedence over reminders and birthdays, even when those come earlier.
const CalendarEvent& SelectHeadline(const QList<CalendarEvent>& events) {
	const auto firstEvent = std::find_if(events.begin(), events.end(), [](const CalendarEvent& event) {
		return event.kind == CalendarEventKind::Event;
	});
	return firstEvent != events.end() ? *firstEvent : events.first();
}
}

CalendarModel::CalendarModel(ICalendarProvider& provider, const IClock& clock, QObject* pParent)
	: QObject(pParent)
	, m_provider(provider)
	, m_clock(clock) {
	m_timer.setInterval(refreshIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &CalendarModel::Refresh);
	Refresh();
}

// Returns the title of the headline entry, or "No events".
const QString& CalendarModel::TitleText() const {
	return m_titleText;
}

// Returns the date and time of the headline entry, or an empty text.
const QString& CalendarModel::WhenText() const {
	return m_whenText;
}

// Returns all upcoming entries as maps with title, whenText and kind for the list screen.
const QVariantList& CalendarModel::Entries() const {
	return m_entries;
}

// Returns whether the calendar account has to be linked before events can be shown.
bool CalendarModel::NeedsLinking() const {
	return m_provider.NeedsLinking();
}

// Asks the provider to start linking the calendar account.
void CalendarModel::BeginLinking() {
	m_provider.BeginLinking();
}

// Starts refreshing the next event periodically.
void CalendarModel::Start() {
	m_timer.start();
}

// Asks the provider for the upcoming entries and notifies listeners when the displayed data changed.
void CalendarModel::Refresh() {
	const QList<CalendarEvent> events = m_provider.UpcomingEvents(m_clock.Now());
	const bool needsLinking = m_provider.NeedsLinking();

	QString titleText = QStringLiteral("No events");
	QString whenText;
	if (needsLinking) {
		titleText = QStringLiteral("Link Google");
		whenText = QStringLiteral("Tap to connect");
	} else if (!events.isEmpty()) {
		const CalendarEvent& headline = SelectHeadline(events);
		titleText = headline.title;
		whenText = FormatWhen(headline);
	}
	const QVariantList entries = needsLinking ? QVariantList() : ToEntries(events);

	if (titleText == m_titleText && whenText == m_whenText && needsLinking == m_needsLinking && entries == m_entries) {
		return;
	}

	m_titleText = titleText;
	m_whenText = whenText;
	m_needsLinking = needsLinking;
	m_entries = entries;
	emit Changed();
}
