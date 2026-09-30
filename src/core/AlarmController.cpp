#include "AlarmController.h"

namespace {
constexpr int checkIntervalMilliseconds = 1000;
}

AlarmController::AlarmController(const IClock& clock, ILightController& light, const AlarmSettings& settings, QObject* pParent)
	: QObject(pParent)
	, m_clock(clock)
	, m_light(light)
	, m_settings(settings) {
	m_timer.setInterval(checkIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &AlarmController::Check);
}

// Returns the alarm time as HH:mm.
QString AlarmController::TimeText() const {
	return m_settings.time.toString(QStringLiteral("HH:mm"));
}

// Returns seven booleans, Monday first, telling on which weekdays the alarm is active.
QVariantList AlarmController::ActiveDays() const {
	QVariantList days;
	for (const bool isActive : m_settings.activeDays) {
		days.append(isActive);
	}
	return days;
}

// Starts checking the clock periodically.
void AlarmController::Start() {
	m_timer.start();
}

// Switches the light on when the alarm minute of an active day is reached for the first time that day.
void AlarmController::Check() {
	const QDateTime now = m_clock.Now();

	if (!IsActiveOn(now.date()) || !IsAlarmMinute(now.time())) {
		return;
	}

	if (m_lastTriggerDate == now.date()) {
		return;
	}

	m_lastTriggerDate = now.date();
	m_light.SetOn(true);
	emit Triggered();
}

// Returns whether the alarm is enabled for the weekday of the given date.
bool AlarmController::IsActiveOn(const QDate& date) const {
	return m_settings.activeDays.at(date.dayOfWeek() - 1);
}

// Returns whether the given time lies within the alarm minute.
bool AlarmController::IsAlarmMinute(const QTime& time) const {
	return time.hour() == m_settings.time.hour() && time.minute() == m_settings.time.minute();
}
