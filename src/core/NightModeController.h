#pragma once

#include "IClock.h"
#include "ILightController.h"
#include "NightModeSettings.h"

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

// Darkens the display inside the night window after a period without input, unless the light is on.
class NightModeController : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool isActive READ IsActive NOTIFY ActiveChanged)
	Q_PROPERTY(bool isEnabled READ IsEnabled NOTIFY SettingsChanged)
	Q_PROPERTY(QString startTimeText READ StartTimeText NOTIFY SettingsChanged)
	Q_PROPERTY(QString endTimeText READ EndTimeText NOTIFY SettingsChanged)

public:
	static constexpr int idleTimeoutSeconds = 5 * 60;

	NightModeController(const IClock& clock, ILightController& light, const NightModeSettings& settings, QObject* pParent = nullptr);

	bool IsActive() const;
	bool IsEnabled() const;
	QString StartTimeText() const;
	QString EndTimeText() const;
	const NightModeSettings& Settings() const;

	void Start();
	void Check();
	void RegisterInput();
	Q_INVOKABLE void Activate();
	Q_INVOKABLE void Wake();
	Q_INVOKABLE void SetEnabled(bool isEnabled);
	Q_INVOKABLE void ShiftStartTime(int minutes);
	Q_INVOKABLE void ShiftEndTime(int minutes);

signals:
	void ActiveChanged();
	void SettingsChanged();

private:
	bool IsInsideNightWindow(const QTime& time) const;
	void SetActive(bool isActive);

	const IClock& m_clock;
	ILightController& m_light;
	NightModeSettings m_settings;
	QDateTime m_lastInput;
	bool m_isActive = false;
	QTimer m_timer;
};
