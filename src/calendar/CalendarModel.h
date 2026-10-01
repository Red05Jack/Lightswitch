#pragma once

#include "ICalendarProvider.h"
#include "IClock.h"

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>

// Exposes the headline entry for the calendar tile and the list of all upcoming entries, or the request to link the account.
class CalendarModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString titleText READ TitleText NOTIFY Changed)
	Q_PROPERTY(QString whenText READ WhenText NOTIFY Changed)
	Q_PROPERTY(QVariantList entries READ Entries NOTIFY Changed)
	Q_PROPERTY(bool needsLinking READ NeedsLinking NOTIFY Changed)

public:
	CalendarModel(ICalendarProvider& provider, const IClock& clock, QObject* pParent = nullptr);

	const QString& TitleText() const;
	const QString& WhenText() const;
	const QVariantList& Entries() const;
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
	QVariantList m_entries;
	bool m_needsLinking = false;
};
