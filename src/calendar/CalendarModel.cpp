#include "CalendarModel.h"

namespace {
constexpr int refreshIntervalMilliseconds = 60 * 1000;
}

CalendarModel::CalendarModel(const ICalendarProvider& provider, const IClock& clock, QObject* pParent)
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

// Starts refreshing the next event periodically.
void CalendarModel::Start() {
	m_timer.start();
}

// Asks the provider for the next event and notifies listeners when the displayed texts changed.
void CalendarModel::Refresh() {
	const QList<CalendarEvent> events = m_provider.UpcomingEvents(m_clock.Now());

	QString titleText = QStringLiteral("No events");
	QString whenText;
	if (!events.isEmpty()) {
		const CalendarEvent& nextEvent = events.first();
		titleText = nextEvent.title;
		whenText = nextEvent.start.toString(QStringLiteral("dd.MM.yyyy")) + QChar(0x2022)
			+ nextEvent.start.toString(QStringLiteral("HH:mm"));
	}

	if (titleText == m_titleText && whenText == m_whenText) {
		return;
	}

	m_titleText = titleText;
	m_whenText = whenText;
	emit Changed();
}
