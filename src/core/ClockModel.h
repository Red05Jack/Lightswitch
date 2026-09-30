#pragma once

#include "IClock.h"

#include <QObject>
#include <QString>
#include <QTimer>

// Exposes the formatted time and date for display and refreshes them every second.
class ClockModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString timeText READ TimeText NOTIFY Changed)
	Q_PROPERTY(QString dateText READ DateText NOTIFY Changed)

public:
	explicit ClockModel(const IClock& clock, QObject* pParent = nullptr);

	const QString& TimeText() const;
	const QString& DateText() const;

	void Start();
	void Refresh();

signals:
	void Changed();

private:
	const IClock& m_clock;
	QTimer m_timer;
	QString m_timeText;
	QString m_dateText;
};
