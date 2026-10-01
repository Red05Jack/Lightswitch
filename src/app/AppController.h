#pragma once

#include "AlarmController.h"
#include "CalendarModel.h"
#include "ClockModel.h"
#include "Configuration.h"
#include "DummyCalendarProvider.h"
#include "GoogleCalendarProvider.h"
#include "DummyLightController.h"
#include "InputActivityMonitor.h"
#include "LocationController.h"
#include "NightModeController.h"
#include "SystemClock.h"
#include "WeatherModel.h"
#include "WeatherService.h"

#include <QObject>
#include <QUrl>
#include <QString>
#include <QTimer>

#include <memory>

// Owns and wires all models and services, persists changed settings and exposes everything to the QML user interface.
class AppController : public QObject {
	Q_OBJECT
	Q_PROPERTY(QObject* clock READ Clock CONSTANT)
	Q_PROPERTY(QObject* light READ Light CONSTANT)
	Q_PROPERTY(QObject* alarm READ Alarm CONSTANT)
	Q_PROPERTY(QObject* nightMode READ NightMode CONSTANT)
	Q_PROPERTY(QObject* location READ Location CONSTANT)
	Q_PROPERTY(QObject* weather READ Weather CONSTANT)
	Q_PROPERTY(QObject* calendar READ Calendar CONSTANT)
	Q_PROPERTY(bool isFullscreen READ IsFullscreen CONSTANT)

public:
	AppController(const Configuration& configuration, const QString& configurationPath, bool isFullscreen, QObject* pParent = nullptr);
	~AppController() override;

	QObject* Clock();
	QObject* Light();
	QObject* Alarm();
	QObject* NightMode();
	QObject* Location();
	QObject* Weather();
	QObject* Calendar();
	bool IsFullscreen() const;
	InputActivityMonitor& InputMonitor();

	void Start();

signals:
	void OpenUrlRequested(const QUrl& url);

private:
	static std::unique_ptr<ICalendarProvider> CreateCalendarProvider(const QString& configurationPath, GoogleCalendarProvider*& pGoogleProvider);

	void SaveConfiguration();

	Configuration m_configuration;
	QString m_configurationPath;
	QTimer m_saveTimer;
	SystemClock m_systemClock;
	DummyLightController m_light;
	ClockModel m_clockModel;
	AlarmController m_alarmController;
	NightModeController m_nightModeController;
	InputActivityMonitor m_inputMonitor;
	LocationController m_locationController;
	WeatherModel m_weatherModel;
	WeatherService m_weatherService;
	GoogleCalendarProvider* m_pGoogleProvider = nullptr;
	std::unique_ptr<ICalendarProvider> m_calendarProvider;
	CalendarModel m_calendarModel;
	bool m_isFullscreen;
};
