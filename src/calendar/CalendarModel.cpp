#include "CalendarModel.h"

namespace {
constexpr int refreshIntervalMilliseconds = 60 * 1000;
}

CalendarModel::CalendarModel(ICalendarProvider& provider, const IClock& clock, QObject* pParent)
	: QObject(pParent)
	, m_provider(provider)
	, m_clock(clock) {
	m_timer.setInterval(refreshIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &CalendarModel::Refresh);
	Refresh();
}

// Returns the title of the next event, or "No events".
const QString& CalendarModel::TitleText() const {
	return m_titleText;
}

// Returns the date and time of the next event, or an empty text.
const QString& CalendarModel::WhenText() const {
	return m_whenText;
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

// Asks the provider for the next event and notifies listeners when the displayed texts changed.
void CalendarModel::Refresh() {
	const QList<CalendarEvent> events = m_provider.UpcomingEvents(m_clock.Now());
	const bool needsLinking = m_provider.NeedsLinking();

	QString titleText = QStringLiteral("No events");
	QString whenText;
	if (needsLinking) {
		titleText = QStringLiteral("Link Google");
		whenText = QStringLiteral("Tap to connect");
	} else if (!events.isEmpty()) {
		const CalendarEvent& nextEvent = events.first();
		titleText = nextEvent.title;
		whenText = nextEvent.start.toString(QStringLiteral("dd.MM.yyyy"));
		if (!nextEvent.isAllDay) {
			whenText += QChar(0x2022) + nextEvent.start.toString(QStringLiteral("HH:mm"));
		}
	}

	if (titleText == m_titleText && whenText == m_whenText && needsLinking == m_needsLinking) {
		return;
	}

	m_titleText = titleText;
	m_whenText = whenText;
	m_needsLinking = needsLinking;
	emit Changed();
}
