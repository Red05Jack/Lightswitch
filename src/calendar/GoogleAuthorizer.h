#pragma once

#include "GoogleCredentials.h"

#include <QByteArray>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

class QNetworkReply;
class QTcpSocket;

// Links the app to a Google account (OAuth authorization code flow with PKCE and a loopback redirect)
// and hands out short lived access tokens; the long lived refresh token is stored in a local file.
class GoogleAuthorizer : public QObject {
	Q_OBJECT

public:
	static QString CodeChallenge(const QString& codeVerifier);

	GoogleAuthorizer(const GoogleCredentials& credentials, const QString& tokenFilePath, QObject* pParent = nullptr);

	bool IsLinked() const;

	void RequestAccessToken();
	void BeginLinking();

signals:
	void AccessTokenReady(const QString& accessToken);
	void RequestFailed();
	void LinkedChanged();
	void AuthorizationUrlReady(const QUrl& url);

private:
	QNetworkReply* PostForm(const QUrlQuery& form);
	void HandleRefreshReply(QNetworkReply* pReply);
	void HandleExchangeReply(QNetworkReply* pReply);
	void HandleConnection();
	void HandleRequest(QTcpSocket* pSocket, const QByteArray& requestLine);
	void ExchangeCode(const QString& code);
	void StopLinking();
	void StoreRefreshToken(const QString& refreshToken);
	void ClearRefreshToken();
	QUrl BuildAuthorizationUrl() const;

	GoogleCredentials m_credentials;
	QString m_tokenFilePath;
	QString m_refreshToken;
	QString m_accessToken;
	QDateTime m_accessTokenExpiry;
	QNetworkAccessManager m_network;
	QTcpServer m_server;
	QTimer m_linkingTimeout;
	QString m_codeVerifier;
	QString m_state;
	QString m_redirectUri;
};
