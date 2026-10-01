#include "GoogleCredentials.h"

#include <QSettings>

// Reads clientId and clientSecret from the [google] section of the file; missing entries stay empty.
GoogleCredentials GoogleCredentials::Load(const QString& filePath) {
	const QSettings settings(filePath, QSettings::IniFormat);

	GoogleCredentials credentials;
	credentials.clientId = settings.value(QStringLiteral("google/clientId")).toString().trimmed();
	credentials.clientSecret = settings.value(QStringLiteral("google/clientSecret")).toString().trimmed();
	return credentials;
}

// Returns whether a client id is present; the secret is optional for some client types.
bool GoogleCredentials::IsValid() const {
	return !clientId.isEmpty();
}
