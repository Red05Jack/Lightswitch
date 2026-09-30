#include "AppController.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QRandomGenerator>

Q_LOGGING_CATEGORY(lcApp, "lightswitch.app")

namespace {
constexpr int saveDelayMilliseconds = 1000;
const QString googleCredentialsFileName = QStringLiteral("google.ini");
const QString googleTokenFileName = QStringLiteral("google-token.ini");
}

AppController::AppController(const Configuration& configuration, const QString& configurationPath, bool isFullscreen, QObject* pParent)
	: QObject(pParent)
	, m_configuration(configuration)
	, m_configurationPath(configurationPath)
	, m_clockModel(m_systemClock)
	, m_alarmController(m_systemClock, m_light, configuration.Alarm())
	, m_nightModeController(m_systemClock, m_light, configuration.NightMode())
	, m_inputMonitor(m_nightModeController)
	, m_locationController(configuration.Location())
	, m_weatherService(configuration.Location().latitude, configuration.Location().longitude)
	, m_calendarProvider(CreateCalendarProvider(configurationPath, m_pGoogleProvider))
	, m_calendarModel(*m_calendarProvider, m_systemClock)
	, m_isFullscreen(isFullscreen) {
	// Changes are written after a short delay so that repeated taps cause only one file write.
	m_saveTimer.setSingleShot(true);
	m_saveTimer.setInterval(saveDelayMilliseconds);
	connect(&m_saveTimer, &QTimer::timeout, this, &AppController::SaveConfiguration);

	connect(&m_weatherService, &WeatherService::ForecastReady, &m_weatherModel, &WeatherModel::SetForecast);
	connect(&m_alarmController, &AlarmController::Triggered, &m_nightModeController, &NightModeController::Wake);
	connect(&m_alarmController, &AlarmController::SettingsChanged, &m_saveTimer, qOverload<>(&QTimer::start));
	connect(&m_nightModeController, &NightModeController::SettingsChanged, &m_saveTimer, qOverload<>(&QTimer::start));
	connect(&m_locationController, &LocationController::LocationChanged, &m_saveTimer, qOverload<>(&QTimer::start));
	if (m_pGoogleProvider != nullptr) {
		connect(m_pGoogleProvider, &GoogleCalendarProvider::Changed, &m_calendarModel, &CalendarModel::Refresh);
		connect(m_pGoogleProvider, &GoogleCalendarProvider::AuthorizationUrlReady, this, &AppController::OpenUrlRequested);
	}
	connect(&m_locationController, &LocationController::LocationChanged, this, [this](const LocationSettings& location) {
		m_weatherService.SetLocation(location.latitude, location.longitude);
	});
}

// Uses Google Calendar when google.ini next to the configuration file holds a client id, otherwise random dummy events.
std::unique_ptr<ICalendarProvider> AppController::CreateCalendarProvider(const QString& configurationPath, GoogleCalendarProvider*& pGoogleProvider) {
	const QDir directory = QFileInfo(configurationPath).absoluteDir();
	const GoogleCredentials credentials = GoogleCredentials::Load(directory.filePath(googleCredentialsFileName));
	if (!credentials.IsValid()) {
		qCInfo(lcApp) << "No Google client id found, showing dummy calendar events.";
		return std::make_unique<DummyCalendarProvider>(QRandomGenerator::global()->generate());
	}

	auto provider = std::make_unique<GoogleCalendarProvider>(credentials, directory.filePath(googleTokenFileName));
	pGoogleProvider = provider.get();
	return provider;
}

// Writes pending settings so that a change made right before shutdown is not lost.
AppController::~AppController() {
	if (m_saveTimer.isActive()) {
		SaveConfiguration();
	}
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

// Returns the location controller for QML.
QObject* AppController::Location() {
	return &m_locationController;
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
	if (m_pGoogleProvider != nullptr) {
		m_pGoogleProvider->Start();
	}
	m_weatherService.Start();
}

// Collects the current settings of all controllers and writes them to the configuration file.
void AppController::SaveConfiguration() {
	m_saveTimer.stop();
	m_configuration.SetLocation(m_locationController.Location());
	m_configuration.SetAlarm(m_alarmController.Settings());
	m_configuration.SetNightMode(m_nightModeController.Settings());

	if (!m_configuration.Save(m_configurationPath)) {
		qCWarning(lcApp) << "Could not save configuration to" << m_configurationPath;
	}
}
