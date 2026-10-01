#pragma once

#include <QMetaType>
#include <QString>

// A named place on earth used for the weather forecast.
struct LocationSettings {
	QString name = QStringLiteral("Graz");
	double latitude = 47.0707;
	double longitude = 15.4395;
};

Q_DECLARE_METATYPE(LocationSettings)
