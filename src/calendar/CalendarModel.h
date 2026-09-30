#pragma once

#include "IClock.h"
#include "ICalendarProvider.h"

#include <QObject>
#include <QString>
#include <QTimer>

// Exposes the next upcoming calendar event as text for the calendar tile.
class CalendarModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString titleText READ TitleText NOTIFY Changed)
	Q_PROPERTY(QString whenText READ WhenText NOTIFY Changed)

public:
	CalendarModel(const ICalendarProvider& provider, const IClock& clock, QObject* pParent = nullptr);

	const QString& TitleText() const;
	const QString& WhenText() const;

	void Start();
	void Refresh();

signals:
	void Changed();

private:
	const ICalendarProvider& m_provider;
	const IClock& m_clock;
	QTimer m_timer;
	QString m_titleText;
	QString m_whenText;
};
