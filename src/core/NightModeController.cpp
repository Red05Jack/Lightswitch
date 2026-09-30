#include "NightModeController.h"

namespace {
constexpr int checkIntervalMilliseconds = 1000;
}

NightModeController::NightModeController(const IClock& clock, ILightController& light, const NightModeSettings& settings, QObject* pParent)
	: QObject(pParent)
	, m_clock(clock)
	, m_light(light)
	, m_settings(settings)
	, m_lastInput(clock.Now()) {
	m_timer.setInterval(checkIntervalMilliseconds);
	connect(&m_timer, &QTimer::timeout, this, &NightModeController::Check);
	connect(&m_light, &ILightController::StateChanged, this, [this](bool isOn) {
		if (isOn) {
			Wake();
		}
	});
}

// Returns whether the display is currently dark.
bool NightModeController::IsActive() const {
	return m_isActive;
}

// Returns the night mode schedule.
const NightModeSettings& NightModeController::Settings() const {
	return m_settings;
}

// Starts checking the idle time periodically.
void NightModeController::Start() {
	m_timer.start();
}

// Goes dark when night mode is enabled, the light is off, the time lies in the night window and nobody touched the screen lately.
void NightModeController::Check() {
	if (m_isActive || !m_settings.isEnabled || m_light.IsOn()) {
		return;
	}

	const QDateTime now = m_clock.Now();
	if (!IsInsideNightWindow(now.time()) || m_lastInput.secsTo(now) < idleTimeoutSeconds) {
		return;
	}

	SetActive(true);
}

// Restarts the idle period; called for every user input.
void NightModeController::RegisterInput() {
	m_lastInput = m_clock.Now();
}

// Goes dark immediately, independent of the schedule.
void NightModeController::Activate() {
	SetActive(true);
}

// Shows the normal display again and restarts the idle period.
void NightModeController::Wake() {
	RegisterInput();
	SetActive(false);
}

// Returns whether the time lies between start and end time; an empty window (start equals end) never matches.
bool NightModeController::IsInsideNightWindow(const QTime& time) const {
	const QTime& start = m_settings.startTime;
	const QTime& end = m_settings.endTime;
	if (start < end) {
		return time >= start && time < end;
	}
	return start != end && (time >= start || time < end);
}

// Stores the new state and notifies listeners when it changed.
void NightModeController::SetActive(bool isActive) {
	if (m_isActive == isActive) {
		return;
	}

	m_isActive = isActive;
	emit ActiveChanged();
}
