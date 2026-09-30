#pragma once

#include "ICalendarProvider.h"
#include "IClock.h"

#include <QObject>
#include <QString>
#include <QTimer>

// Exposes the next upcoming calendar event, or the request to link the account, as text for the calendar tile.
class CalendarModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString titleText READ TitleText NOTIFY Changed)
	Q_PROPERTY(QString whenText READ WhenText NOTIFY Changed)
	Q_PROPERTY(bool needsLinking READ NeedsLinking NOTIFY Changed)

public:
	CalendarModel(ICalendarProvider& provider, const IClock& clock, QObject* pParent = nullptr);

	const QString& TitleText() const;
	const QString& WhenText() const;
	bool NeedsLinking() const;

	void Start();
	void Refresh();
	Q_INVOKABLE void BeginLinking();

signals:
	void Changed();

private:
	ICalendarProvider& m_provider;
	const IClock& m_clock;
	QTimer m_timer;
	QString m_titleText;
	QString m_whenText;
	bool m_needsLinking = false;
};
