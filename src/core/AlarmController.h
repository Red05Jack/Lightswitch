#pragma once

#include "AlarmSettings.h"
#include "IClock.h"
#include "ILightController.h"

#include <QDate>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>

// Switches the light on at the configured time on the configured weekdays, once per day.
class AlarmController : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString timeText READ TimeText CONSTANT)
	Q_PROPERTY(QVariantList activeDays READ ActiveDays CONSTANT)

public:
	AlarmController(const IClock& clock, ILightController& light, const AlarmSettings& settings, QObject* pParent = nullptr);

	QString TimeText() const;
	QVariantList ActiveDays() const;

	void Start();
	void Check();

signals:
	void Triggered();

private:
	bool IsActiveOn(const QDate& date) const;
	bool IsAlarmMinute(const QTime& time) const;

	const IClock& m_clock;
	ILightController& m_light;
	AlarmSettings m_settings;
	QDate m_lastTriggerDate;
	QTimer m_timer;
};
