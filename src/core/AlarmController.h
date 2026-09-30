#pragma once

#include "AlarmSettings.h"
#include "IClock.h"
#include "ILightController.h"

#include <QDate>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>

// Switches the light on at the time configured for the weekday, once per day, unless the alarm is switched off globally.
class AlarmController : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool isEnabled READ IsEnabled NOTIFY Changed)
	Q_PROPERTY(QString nextAlarmText READ NextAlarmText NOTIFY Changed)
	Q_PROPERTY(QVariantList activeDays READ ActiveDays NOTIFY Changed)
	Q_PROPERTY(QVariantList timeTexts READ TimeTexts NOTIFY Changed)

public:
	AlarmController(const IClock& clock, ILightController& light, const AlarmSettings& settings, QObject* pParent = nullptr);

	bool IsEnabled() const;
	const QString& NextAlarmText() const;
	QVariantList ActiveDays() const;
	QVariantList TimeTexts() const;
	const AlarmSettings& Settings() const;

	void Start();
	void Check();
	Q_INVOKABLE void SetEnabled(bool isEnabled);
	Q_INVOKABLE void SetDayActive(int dayIndex, bool isActive);
	Q_INVOKABLE void ShiftDayTime(int dayIndex, int minutes);

signals:
	void Triggered();
	void Changed();
	void SettingsChanged();

private:
	const AlarmDay& DayOf(const QDate& date) const;
	bool IsAlarmMinute(const AlarmDay& day, const QTime& time) const;
	QString ComputeNextAlarmText() const;
	void UpdateNextAlarmText();
	void NotifySettingsChanged();

	const IClock& m_clock;
	ILightController& m_light;
	AlarmSettings m_settings;
	QString m_nextAlarmText;
	QDate m_lastTriggerDate;
	QTimer m_timer;
};
