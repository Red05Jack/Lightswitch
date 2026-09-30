#include "AppController.h"

#include <QRandomGenerator>

AppController::AppController(const Configuration& configuration, bool isFullscreen, QObject* pParent)
	: QObject(pParent)
	, m_clockModel(m_systemClock)
	, m_alarmController(m_systemClock, m_light, configuration.Alarm())
	, m_weatherService(configuration.Latitude(), configuration.Longitude())
	, m_calendarProvider(QRandomGenerator::global()->generate())
	, m_calendarModel(m_calendarProvider, m_systemClock)
	, m_isFullscreen(isFullscreen) {
	connect(&m_weatherService, &WeatherService::ForecastReady, &m_weatherModel, &WeatherModel::SetForecast);
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

// Starts all periodic updates.
void AppController::Start() {
	m_clockModel.Start();
	m_alarmController.Start();
	m_calendarModel.Start();
	m_weatherService.Start();
}
