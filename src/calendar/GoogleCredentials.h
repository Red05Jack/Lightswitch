#pragma once

#include <QString>

// OAuth client of the Google Cloud project (type "Desktop app"), read from a separate file so that it is never committed.
struct GoogleCredentials {
	static GoogleCredentials Load(const QString& filePath);

	bool IsValid() const;

	QString clientId;
	QString clientSecret;
};
