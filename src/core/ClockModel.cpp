#include "ClockModel.h"

#include <QLocale>

namespace {
constexpr int refreshIntervalMilliseconds = 1000;
}

ClockModel::ClockModel(const IClock& clock, QObject* pParent)
	: QObject(pParent)
	, m_clock(clock) {
	m_timer.setInterval(refreshIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &ClockModel::Refresh);
	Refresh();
}

// Returns the time as HH:mm.
const QString& ClockModel::TimeText() const {
	return m_timeText;
}

// Returns the date such as "Thursday sep 17".
const QString& ClockModel::DateText() const {
	return m_dateText;
}

// Starts refreshing the texts periodically.
void ClockModel::Start() {
	m_timer.start();
}

// Re-reads the clock and notifies listeners when a displayed text has changed.
void ClockModel::Refresh() {
	const QDateTime now = m_clock.Now();
	const QDate date = now.date();
	const QLocale english(QLocale::English);

	const QString timeText = now.toString(QStringLiteral("HH:mm"));
	const QString dateText = english.toString(date, QStringLiteral("dddd")) + QLatin1Char(' ')
		+ english.toString(date, QStringLiteral("MMM")).toLower() + QLatin1Char(' ')
		+ QString::number(date.day());

	if (timeText == m_timeText && dateText == m_dateText) {
		return;
	}

	m_timeText = timeText;
	m_dateText = dateText;
	emit Changed();
}
