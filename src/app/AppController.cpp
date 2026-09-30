#include "AppController.h"

#include <QRandomGenerator>

AppController::AppController(const Configuration& configuration, bool isFullscreen, QObject* pParent)
	: QObject(pParent)
	, m_clockModel(m_systemClock)
	, m_alarmController(m_systemClock, m_light, configuration.Alarm())
	, m_nightModeController(m_systemClock, m_light, configuration.NightMode())
	, m_inputMonitor(m_nightModeController)
	, m_weatherService(configuration.Latitude(), configuration.Longitude())
	, m_calendarProvider(QRandomGenerator::global()->generate())
	, m_calendarModel(m_calendarProvider, m_systemClock)
	, m_isFullscreen(isFullscreen) {
	connect(&m_weatherService, &WeatherService::ForecastReady, &m_weatherModel, &WeatherModel::SetForecast);
	connect(&m_alarmController, &AlarmController::Triggered, &m_nightModeController, &NightModeController::Wake);
}

// Returns the clock model for QML.
QObject* AppController::Clock() {
	return &m_clockModel;
}

// Returns the light controller for QML.
QObject* AppController::Light() {
	return &m_light;
}

// Returns the alarm controller for QML.
QObject* AppController::Alarm() {
	return &m_alarmController;
}

// Returns the night mode controller for QML.
QObject* AppController::NightMode() {
	return &m_nightModeController;
}

// Returns the weather model for QML.
QObject* AppController::Weather() {
	return &m_weatherModel;
}

// Returns the calendar model for QML.
QObject* AppController::Calendar() {
	return &m_calendarModel;
}

// Returns whether the window should cover the whole screen.
bool AppController::IsFullscreen() const {
	return m_isFullscreen;
}

// Returns the event filter that has to be installed on the application.
InputActivityMonitor& AppController::InputMonitor() {
	return m_inputMonitor;
}

// Starts all periodic updates.
void AppController::Start() {
	m_clockModel.Start();
	m_alarmController.Start();
	m_nightModeController.Start();
	m_calendarModel.Start();
	m_weatherService.Start();
}
