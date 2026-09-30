#include "Configuration.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

namespace {
// Writes an INI file with the given content into the directory and returns its path.
QString WriteIniFile(const QTemporaryDir& directory, const QByteArray& content) {
	const QString path = directory.filePath(QStringLiteral("lightswitch.ini"));
	QFile file(path);
	if (file.open(QIODevice::WriteOnly)) {
		file.write(content);
	}
	return path;
}

using Days = std::array<bool, 7>;
}

class ConfigurationTest : public QObject {
	Q_OBJECT

private slots:
	void LoadsValidFile();
	void MissingFileUsesDefaults();
	void EmptyFileUsesDefaults();
	void InvalidValuesUseDefaults();
	void ParsesActiveDays();
};

void ConfigurationTest::LoadsValidFile() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory,
		"[location]\nlatitude=52.52\nlongitude=13.405\n\n[alarm]\ntime=07:30\ndays=Mon,Wed,Sun\n");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Latitude(), 52.52);
	QCOMPARE(configuration.Longitude(), 13.405);
	QCOMPARE(configuration.Alarm().time, QTime(7, 30));
	QCOMPARE(configuration.Alarm().activeDays, (Days{true, false, true, false, false, false, true}));
}

void ConfigurationTest::MissingFileUsesDefaults() {
	QTemporaryDir directory;

	const Configuration configuration = Configuration::Load(directory.filePath(QStringLiteral("missing.ini")));

	QCOMPARE(configuration.Latitude(), 48.1374);
	QCOMPARE(configuration.Longitude(), 11.5755);
	QCOMPARE(configuration.Alarm().time, QTime(6, 45));
	QCOMPARE(configuration.Alarm().activeDays, (Days{true, true, true, true, true, false, false}));
}

void ConfigurationTest::EmptyFileUsesDefaults() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory, "");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Latitude(), 48.1374);
	QCOMPARE(configuration.Alarm().time, QTime(6, 45));
}

void ConfigurationTest::InvalidValuesUseDefaults() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory,
		"[location]\nlatitude=abc\nlongitude=500\n\n[alarm]\ntime=25:99\ndays=xyz\n");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Latitude(), 48.1374);
	QCOMPARE(configuration.Longitude(), 11.5755);
	QCOMPARE(configuration.Alarm().time, QTime(6, 45));
	QCOMPARE(configuration.Alarm().activeDays, (Days{false, false, false, false, false, false, false}));
}

void ConfigurationTest::ParsesActiveDays() {
	QCOMPARE(Configuration::ParseActiveDays(QStringLiteral(" mon , FRI, xyz")),
		(Days{true, false, false, false, true, false, false}));
	QCOMPARE(Configuration::ParseActiveDays(QStringLiteral("Saturday,Sunday")),
		(Days{false, false, false, false, false, true, true}));
	QCOMPARE(Configuration::ParseActiveDays(QString()),
		(Days{false, false, false, false, false, false, false}));
}

QTEST_GUILESS_MAIN(ConfigurationTest)
#include "ConfigurationTest.moc"
