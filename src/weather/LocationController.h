#pragma once

#include "LocationSettings.h"

#include <QList>
#include <QObject>
#include <QString>

// Holds the selected weather location and lets the user step through a list of preset cities.
class LocationController : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString name READ Name NOTIFY LocationChanged)

public:
	static const QList<LocationSettings>& Presets();

	explicit LocationController(const LocationSettings& location, QObject* pParent = nullptr);

	QString Name() const;
	const LocationSettings& Location() const;

	Q_INVOKABLE void SelectNext();
	Q_INVOKABLE void SelectPrevious();

signals:
	void LocationChanged(const LocationSettings& location);

private:
	void SelectPresetOffset(int offset);

	LocationSettings m_location;
};
