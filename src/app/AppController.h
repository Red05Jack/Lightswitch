#pragma once

#include "AlarmController.h"
#include "CalendarModel.h"
#include "ClockModel.h"
#include "Configuration.h"
#include "DummyCalendarProvider.h"
#include "DummyLightController.h"
#include "InputActivityMonitor.h"
#include "NightModeController.h"
#include "SystemClock.h"
#include "WeatherModel.h"
#include "WeatherService.h"

#include <QObject>

// Owns and wires all models and services and exposes them to the QML user interface.
class AppController : public QObject {
	Q_OBJECT
	Q_PROPERTY(QObject* clock READ Clock CONSTANT)
	Q_PROPERTY(QObject* light READ Light CONSTANT)
	Q_PROPERTY(QObject* alarm READ Alarm CONSTANT)
	Q_PROPERTY(QObject* nightMode READ NightMode CONSTANT)
	Q_PROPERTY(QObject* weather READ Weather CONSTANT)
	Q_PROPERTY(QObject* calendar READ Calendar CONSTANT)
	Q_PROPERTY(bool isFullscreen READ IsFullscreen CONSTANT)

public:
	AppController(const Configuration& configuration, bool isFullscreen, QObject* pParent = nullptr);

	QObject* Clock();
	QObject* Light();
	QObject* Alarm();
	QObject* NightMode();
	QObject* Weather();
	QObject* Calendar();
	bool IsFullscreen() const;
	InputActivityMonitor& InputMonitor();

	void Start();

private:
	SystemClock m_systemClock;
	DummyLightController m_light;
	ClockModel m_clockModel;
	AlarmController m_alarmController;
	NightModeController m_nightModeController;
	InputActivityMonitor m_inputMonitor;
	WeatherModel m_weatherModel;
	WeatherService m_weatherService;
	DummyCalendarProvider m_calendarProvider;
	CalendarModel m_calendarModel;
	bool m_isFullscreen;
};
