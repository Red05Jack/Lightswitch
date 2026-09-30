#include "AlarmController.h"

namespace {
constexpr int checkIntervalMilliseconds = 1000;
constexpr int secondsPerMinute = 60;
constexpr int searchDays = 7;
const QString timeFormat = QStringLiteral("HH:mm");
const QString offText = QStringLiteral("OFF");
}

AlarmController::AlarmController(const IClock& clock, ILightController& light, const AlarmSettings& settings, QObject* pParent)
	: QObject(pParent)
	, m_clock(clock)
	, m_light(light)
	, m_settings(settings)
	, m_nextAlarmText(ComputeNextAlarmText()) {
	m_timer.setInterval(checkIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &AlarmController::Check);
}

// Returns whether the alarm is switched on globally.
bool AlarmController::IsEnabled() const {
	return m_settings.isEnabled;
}

// Returns the time of the next alarm as HH:mm, or "OFF" when no alarm will ring.
const QString& AlarmController::NextAlarmText() const {
	return m_nextAlarmText;
}

// Returns seven booleans, Monday first, telling on which weekdays the alarm is active.
QVariantList AlarmController::ActiveDays() const {
	QVariantList days;
	for (const AlarmDay& day : m_settings.days) {
		days.append(day.isActive);
	}
	return days;
}

// Returns seven texts (HH:mm), Monday first, with the alarm time of each weekday.
QVariantList AlarmController::TimeTexts() const {
	QVariantList texts;
	for (const AlarmDay& day : m_settings.days) {
		texts.append(day.time.toString(timeFormat));
	}
	return texts;
}

// Returns the global switch and the alarms of all weekdays.
const AlarmSettings& AlarmController::Settings() const {
	return m_settings;
}

// Starts checking the clock periodically.
void AlarmController::Start() {
	m_timer.start();
}

// Switches the light on when the alarm minute of an active day is reached for the first time that day.
void AlarmController::Check() {
	UpdateNextAlarmText();

	if (!m_settings.isEnabled) {
		return;
	}

	const QDateTime now = m_clock.Now();
	const AlarmDay& today = DayOf(now.date());
	if (!today.isActive || !IsAlarmMinute(today, now.time())) {
		return;
	}

	if (m_lastTriggerDate == now.date()) {
		return;
	}

	m_lastTriggerDate = now.date();
	m_light.SetOn(true);
	emit Triggered();
}

// Switches the whole alarm on or off.
void AlarmController::SetEnabled(bool isEnabled) {
	if (m_settings.isEnabled == isEnabled) {
		return;
	}

	m_settings.isEnabled = isEnabled;
	NotifySettingsChanged();
}

// Activates or deactivates the alarm of one weekday (0 is Monday).
void AlarmController::SetDayActive(int dayIndex, bool isActive) {
	if (dayIndex < 0 || dayIndex >= AlarmSettings::dayCount) {
		return;
	}

	AlarmDay& day = m_settings.days.at(static_cast<size_t>(dayIndex));
	if (day.isActive == isActive) {
		return;
	}

	day.isActive = isActive;
	NotifySettingsChanged();
}

// Moves the alarm time of one weekday (0 is Monday) by the given minutes, wrapping around midnight.
void AlarmController::ShiftDayTime(int dayIndex, int minutes) {
	if (dayIndex < 0 || dayIndex >= AlarmSettings::dayCount) {
		return;
	}

	AlarmDay& day = m_settings.days.at(static_cast<size_t>(dayIndex));
	day.time = day.time.addSecs(minutes * secondsPerMinute);
	NotifySettingsChanged();
}

// Returns the alarm of the weekday of the given date.
const AlarmDay& AlarmController::DayOf(const QDate& date) const {
	return m_settings.days.at(static_cast<size_t>(date.dayOfWeek() - 1));
}

// Returns whether the given time lies within the alarm minute.
bool AlarmController::IsAlarmMinute(const AlarmDay& day, const QTime& time) const {
	return time.hour() == day.time.hour() && time.minute() == day.time.minute();
}

// Finds the first active alarm after now within the next week.
QString AlarmController::ComputeNextAlarmText() const {
	if (!m_settings.isEnabled) {
		return offText;
	}

	const QDateTime now = m_clock.Now();
	for (int dayOffset = 0; dayOffset <= searchDays; ++dayOffset) {
		const QDate date = now.date().addDays(dayOffset);
		const AlarmDay& day = DayOf(date);
		if (day.isActive && QDateTime(date, day.time) > now) {
			return day.time.toString(timeFormat);
		}
	}
	return offText;
}

// Recomputes the next alarm text and notifies listeners when it changed.
void AlarmController::UpdateNextAlarmText() {
	const QString nextAlarmText = ComputeNextAlarmText();
	if (nextAlarmText == m_nextAlarmText) {
		return;
	}

	m_nextAlarmText = nextAlarmText;
	emit Changed();
}

// Refreshes the display after an edit and asks for the settings to be saved.
void AlarmController::NotifySettingsChanged() {
	m_nextAlarmText = ComputeNextAlarmText();
	emit Changed();
	emit SettingsChanged();
}
