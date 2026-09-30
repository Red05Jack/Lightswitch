#include "GoogleAuthorizer.h"

#include "GoogleCalendarParser.h"

#include <QCryptographicHash>
#include <QHostAddress>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QSettings>
#include <QTcpSocket>

Q_LOGGING_CATEGORY(lcGoogleAuth, "lightswitch.calendar.auth")

namespace {
const QString authorizationEndpoint = QStringLiteral("https://accounts.google.com/o/oauth2/v2/auth");
const QString tokenEndpoint = QStringLiteral("https://oauth2.googleapis.com/token");
const QString calendarScope = QStringLiteral("https://www.googleapis.com/auth/calendar.readonly");
const QString refreshTokenKey = QStringLiteral("google/refreshToken");
constexpr int requestTimeoutMilliseconds = 15 * 1000;
constexpr int linkingTimeoutMilliseconds = 10 * 60 * 1000;
constexpr int tokenSafetyMarginSeconds = 60;
constexpr int randomByteCount = 32;
constexpr int httpBadRequest = 400;
constexpr int httpUnauthorized = 401;
constexpr int httpNotFound = 404;
constexpr int httpOk = 200;

// Returns random bytes as URL safe text without padding.
QString RandomText() {
	QByteArray bytes(randomByteCount, 0);
	QRandomGenerator::system()->fillRange(reinterpret_cast<quint32*>(bytes.data()), randomByteCount / static_cast<int>(sizeof(quint32)));
	return QString::fromLatin1(bytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

// Sends a small HTML page to the browser and closes the connection.
void Respond(QTcpSocket* pSocket, int statusCode, const QString& message) {
	const QByteArray body = "<html><body style=\"font-family:sans-serif\"><h2>" + message.toUtf8() + "</h2></body></html>";
	pSocket->write("HTTP/1.1 " + QByteArray::number(statusCode) + " Response\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\nContent-Length: "
		+ QByteArray::number(body.size()) + "\r\n\r\n" + body);
	pSocket->disconnectFromHost();
}
}

// Returns the S256 PKCE challenge of a code verifier.
QString GoogleAuthorizer::CodeChallenge(const QString& codeVerifier) {
	const QByteArray hash = QCryptographicHash::hash(codeVerifier.toLatin1(), QCryptographicHash::Sha256);
	return QString::fromLatin1(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

GoogleAuthorizer::GoogleAuthorizer(const GoogleCredentials& credentials, const QString& tokenFilePath, QObject* pParent)
	: QObject(pParent)
	, m_credentials(credentials)
	, m_tokenFilePath(tokenFilePath) {
	m_refreshToken = QSettings(m_tokenFilePath, QSettings::IniFormat).value(refreshTokenKey).toString();

	connect(&m_server, &QTcpServer::newConnection, this, &GoogleAuthorizer::HandleConnection);
	m_linkingTimeout.setSingleShot(true);
	m_linkingTimeout.setInterval(linkingTimeoutMilliseconds);
	connect(&m_linkingTimeout, &QTimer::timeout, this, &GoogleAuthorizer::StopLinking);
}

// Returns whether a refresh token is stored, meaning the account has been linked.
bool GoogleAuthorizer::IsLinked() const {
	return !m_refreshToken.isEmpty();
}

// Delivers a valid access token, refreshing it when necessary; emits RequestFailed when that is not possible.
void GoogleAuthorizer::RequestAccessToken() {
	if (!IsLinked()) {
		emit RequestFailed();
		return;
	}

	if (!m_accessToken.isEmpty() && QDateTime::currentDateTimeUtc() < m_accessTokenExpiry) {
		emit AccessTokenReady(m_accessToken);
		return;
	}

	QUrlQuery form;
	form.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
	form.addQueryItem(QStringLiteral("refresh_token"), m_refreshToken);
	QNetworkReply* pReply = PostForm(form);
	connect(pReply, &QNetworkReply::finished, this, [this, pReply]() { HandleRefreshReply(pReply); });
}

// Starts the loopback server and publishes the Google sign-in address for the browser.
void GoogleAuthorizer::BeginLinking() {
	if (m_server.isListening()) {
		emit AuthorizationUrlReady(BuildAuthorizationUrl());
		return;
	}

	if (!m_server.listen(QHostAddress::LocalHost, 0)) {
		qCWarning(lcGoogleAuth) << "Could not start the local sign-in server:" << m_server.errorString();
		return;
	}

	m_codeVerifier = RandomText();
	m_state = RandomText();
	m_redirectUri = QStringLiteral("http://127.0.0.1:%1").arg(m_server.serverPort());
	m_linkingTimeout.start();

	const QUrl url = BuildAuthorizationUrl();
	qCWarning(lcGoogleAuth) << "Open this address in a browser on this machine to link Google Calendar:" << url.toString();
	emit AuthorizationUrlReady(url);
}

// Sends the form with the client credentials to the token endpoint.
QNetworkReply* GoogleAuthorizer::PostForm(const QUrlQuery& form) {
	QUrlQuery completeForm = form;
	completeForm.addQueryItem(QStringLiteral("client_id"), m_credentials.clientId);
	if (!m_credentials.clientSecret.isEmpty()) {
		completeForm.addQueryItem(QStringLiteral("client_secret"), m_credentials.clientSecret);
	}

	QNetworkRequest request{QUrl(tokenEndpoint)};
	request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
	request.setTransferTimeout(requestTimeoutMilliseconds);
	return m_network.post(request, completeForm.toString(QUrl::FullyEncoded).toUtf8());
}

// Stores the refreshed access token; a rejected refresh token means the account has to be linked again.
void GoogleAuthorizer::HandleRefreshReply(QNetworkReply* pReply) {
	pReply->deleteLater();

	const int statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	const GoogleTokenResponse response = GoogleCalendarParser::ParseTokenResponse(pReply->readAll());
	if (!response.errorCode.isEmpty()) {
		qCWarning(lcGoogleAuth) << "Token refresh failed:" << response.errorCode << pReply->errorString();
		if (statusCode == httpBadRequest || statusCode == httpUnauthorized) {
			ClearRefreshToken();
		}
		emit RequestFailed();
		return;
	}

	m_accessToken = response.accessToken;
	m_accessTokenExpiry = QDateTime::currentDateTimeUtc().addSecs(response.expiresInSeconds - tokenSafetyMarginSeconds);
	emit AccessTokenReady(m_accessToken);
}

// Stores the refresh token obtained by the code exchange and completes the linking.
void GoogleAuthorizer::HandleExchangeReply(QNetworkReply* pReply) {
	pReply->deleteLater();

	const GoogleTokenResponse response = GoogleCalendarParser::ParseTokenResponse(pReply->readAll());
	if (!response.errorCode.isEmpty() || response.refreshToken.isEmpty()) {
		qCWarning(lcGoogleAuth) << "Linking failed:" << response.errorCode << pReply->errorString();
		return;
	}

	m_accessToken = response.accessToken;
	m_accessTokenExpiry = QDateTime::currentDateTimeUtc().addSecs(response.expiresInSeconds - tokenSafetyMarginSeconds);
	StoreRefreshToken(response.refreshToken);
	qCInfo(lcGoogleAuth) << "Google account linked.";
}

// Reads the request line of a browser connection to the loopback server.
void GoogleAuthorizer::HandleConnection() {
	while (QTcpSocket* pSocket = m_server.nextPendingConnection()) {
		connect(pSocket, &QTcpSocket::disconnected, pSocket, &QObject::deleteLater);
		connect(pSocket, &QTcpSocket::readyRead, this, [this, pSocket]() {
			if (!pSocket->canReadLine()) {
				return;
			}
			HandleRequest(pSocket, pSocket->readLine());
		});
	}
}

// Accepts only the redirect with the expected state; everything else (favicon etc.) gets a 404.
void GoogleAuthorizer::HandleRequest(QTcpSocket* pSocket, const QByteArray& requestLine) {
	const QList<QByteArray> parts = requestLine.split(' ');
	const QUrlQuery query(QUrl(QString::fromLatin1(parts.value(1))));
	const QString code = query.queryItemValue(QStringLiteral("code"));

	if (!query.hasQueryItem(QStringLiteral("code")) && !query.hasQueryItem(QStringLiteral("error"))) {
		Respond(pSocket, httpNotFound, QStringLiteral("Not found"));
		return;
	}

	if (query.queryItemValue(QStringLiteral("state")) != m_state || code.isEmpty()) {
		qCWarning(lcGoogleAuth) << "Sign-in was denied or the state did not match.";
		Respond(pSocket, httpBadRequest, QStringLiteral("Linking failed. Please try again."));
		return;
	}

	Respond(pSocket, httpOk, QStringLiteral("Google Calendar linked. You can close this window."));
	ExchangeCode(code);
	StopLinking();
}

// Trades the authorization code for the tokens.
void GoogleAuthorizer::ExchangeCode(const QString& code) {
	QUrlQuery form;
	form.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));
	form.addQueryItem(QStringLiteral("code"), code);
	form.addQueryItem(QStringLiteral("redirect_uri"), m_redirectUri);
	form.addQueryItem(QStringLiteral("code_verifier"), m_codeVerifier);
	QNetworkReply* pReply = PostForm(form);
	connect(pReply, &QNetworkReply::finished, this, [this, pReply]() { HandleExchangeReply(pReply); });
}

// Closes the loopback server.
void GoogleAuthorizer::StopLinking() {
	m_linkingTimeout.stop();
	m_server.close();
}

// Keeps the refresh token in memory and in the token file.
void GoogleAuthorizer::StoreRefreshToken(const QString& refreshToken) {
	m_refreshToken = refreshToken;
	QSettings settings(m_tokenFilePath, QSettings::IniFormat);
	settings.setValue(refreshTokenKey, refreshToken);
	settings.sync();
	emit LinkedChanged();
}

// Forgets a refresh token that Google no longer accepts.
void GoogleAuthorizer::ClearRefreshToken() {
	m_refreshToken.clear();
	m_accessToken.clear();
	QSettings settings(m_tokenFilePath, QSettings::IniFormat);
	settings.remove(refreshTokenKey);
	settings.sync();
	emit LinkedChanged();
}

// Builds the address the user opens to grant read access to the calendar.
QUrl GoogleAuthorizer::BuildAuthorizationUrl() const {
	QUrlQuery query;
	query.addQueryItem(QStringLiteral("client_id"), m_credentials.clientId);
	query.addQueryItem(QStringLiteral("redirect_uri"), m_redirectUri);
	query.addQueryItem(QStringLiteral("response_type"), QStringLiteral("code"));
	query.addQueryItem(QStringLiteral("scope"), calendarScope);
	query.addQueryItem(QStringLiteral("code_challenge"), CodeChallenge(m_codeVerifier));
	query.addQueryItem(QStringLiteral("code_challenge_method"), QStringLiteral("S256"));
	query.addQueryItem(QStringLiteral("state"), m_state);
	query.addQueryItem(QStringLiteral("access_type"), QStringLiteral("offline"));
	query.addQueryItem(QStringLiteral("prompt"), QStringLiteral("consent"));

	QUrl url{authorizationEndpoint};
	url.setQuery(query);
	return url;
}
