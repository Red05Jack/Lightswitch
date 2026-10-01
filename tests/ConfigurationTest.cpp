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

// Returns the active flags of all weekdays, Monday first.
Days ActiveFlags(const AlarmSettings& alarm) {
	Days flags = {};
	for (size_t dayIndex = 0; dayIndex < flags.size(); ++dayIndex) {
		flags.at(dayIndex) = alarm.days.at(dayIndex).isActive;
	}
	return flags;
}
}

class ConfigurationTest : public QObject {
	Q_OBJECT

private slots:
	void LoadsValidFile();
	void MissingFileUsesDefaults();
	void EmptyFileUsesDefaults();
	void InvalidValuesUseDefaults();
	void ParsesActiveDays();
	void ReadsNightMode();
	void InvalidNightModeUsesDefaults();
	void SavesAndReloadsChangedSettings();
	void ReadsAlarmTimePerWeekday();
	void SavesAlarmTimesAndSwitch();
};

void ConfigurationTest::LoadsValidFile() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory,
		"[location]\nlatitude=52.52\nlongitude=13.405\n\n[alarm]\ntime=07:30\ndays=Mon,Wed,Sun\n");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Location().latitude, 52.52);
	QCOMPARE(configuration.Location().longitude, 13.405);
	QCOMPARE(configuration.Location().name, QStringLiteral("Custom"));
	QCOMPARE(configuration.Alarm().days.at(4).time, QTime(7, 30));
	QCOMPARE(ActiveFlags(configuration.Alarm()), (Days{true, false, true, false, false, false, true}));
}

void ConfigurationTest::ReadsNightMode() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory, "[nightmode]\nenabled=false\nstart=23:15\nend=05:30\n");

	const Configuration configuration = Configuration::Load(path);

	QVERIFY(!configuration.NightMode().isEnabled);
	QCOMPARE(configuration.NightMode().startTime, QTime(23, 15));
	QCOMPARE(configuration.NightMode().endTime, QTime(5, 30));
}

void ConfigurationTest::InvalidNightModeUsesDefaults() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory, "[nightmode]\nstart=99:99\nend=abc\n");

	const Configuration configuration = Configuration::Load(path);

	QVERIFY(configuration.NightMode().isEnabled);
	QCOMPARE(configuration.NightMode().startTime, QTime(22, 0));
	QCOMPARE(configuration.NightMode().endTime, QTime(6, 0));
}

void ConfigurationTest::MissingFileUsesDefaults() {
	QTemporaryDir directory;

	const Configuration configuration = Configuration::Load(directory.filePath(QStringLiteral("missing.ini")));

	QCOMPARE(configuration.Location().name, QStringLiteral("Graz"));
	QCOMPARE(configuration.Location().latitude, 47.0707);
	QCOMPARE(configuration.Location().longitude, 15.4395);
	QCOMPARE(configuration.Alarm().days.at(4).time, QTime(6, 45));
	QCOMPARE(ActiveFlags(configuration.Alarm()), (Days{true, true, true, true, true, false, false}));
}

void ConfigurationTest::EmptyFileUsesDefaults() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory, "");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Location().latitude, 47.0707);
	QCOMPARE(configuration.Alarm().days.at(4).time, QTime(6, 45));
}

void ConfigurationTest::InvalidValuesUseDefaults() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory,
		"[location]\nlatitude=abc\nlongitude=500\n\n[alarm]\ntime=25:99\ndays=xyz\n");

	const Configuration configuration = Configuration::Load(path);

	QCOMPARE(configuration.Location().name, QStringLiteral("Graz"));
	QCOMPARE(configuration.Location().latitude, 47.0707);
	QCOMPARE(configuration.Location().longitude, 15.4395);
	QCOMPARE(configuration.Alarm().days.at(4).time, QTime(6, 45));
	QCOMPARE(ActiveFlags(configuration.Alarm()), (Days{false, false, false, false, false, false, false}));
}

void ConfigurationTest::ParsesActiveDays() {
	QCOMPARE(Configuration::ParseActiveDays(QStringLiteral(" mon , FRI, xyz")),
		(Days{true, false, false, false, true, false, false}));
	QCOMPARE(Configuration::ParseActiveDays(QStringLiteral("Saturday,Sunday")),
		(Days{false, false, false, false, false, true, true}));
	QCOMPARE(Configuration::ParseActiveDays(QString()),
		(Days{false, false, false, false, false, false, false}));
}

void ConfigurationTest::SavesAndReloadsChangedSettings() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory, "[alarm]\ntime=07:30\ndays=Mon,Wed\n");
	Configuration configuration = Configuration::Load(path);
	configuration.SetLocation({QStringLiteral("Vienna"), 48.2082, 16.3738});
	NightModeSettings nightMode;
	nightMode.isEnabled = false;
	nightMode.startTime = QTime(21, 30);
	nightMode.endTime = QTime(7, 15);
	configuration.SetNightMode(nightMode);

	QVERIFY(configuration.Save(path));
	const Configuration reloaded = Configuration::Load(path);

	QCOMPARE(reloaded.Location().name, QStringLiteral("Vienna"));
	QCOMPARE(reloaded.Location().latitude, 48.2082);
	QCOMPARE(reloaded.Location().longitude, 16.3738);
	QVERIFY(!reloaded.NightMode().isEnabled);
	QCOMPARE(reloaded.NightMode().startTime, QTime(21, 30));
	QCOMPARE(reloaded.NightMode().endTime, QTime(7, 15));
	QCOMPARE(reloaded.Alarm().days.at(2).time, QTime(7, 30));
	QCOMPARE(ActiveFlags(reloaded.Alarm()), (Days{true, false, true, false, false, false, false}));
}

void ConfigurationTest::ReadsAlarmTimePerWeekday() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory,
		"[alarm]\nenabled=false\ntime=06:00\nmon=05:30\nsun=10:15\ndays=Mon,Sun\n");

	const Configuration configuration = Configuration::Load(path);

	QVERIFY(!configuration.Alarm().isEnabled);
	QCOMPARE(configuration.Alarm().days.at(0).time, QTime(5, 30));
	QCOMPARE(configuration.Alarm().days.at(1).time, QTime(6, 0));
	QCOMPARE(configuration.Alarm().days.at(6).time, QTime(10, 15));
	QCOMPARE(ActiveFlags(configuration.Alarm()), (Days{true, false, false, false, false, false, true}));
}

void ConfigurationTest::SavesAlarmTimesAndSwitch() {
	QTemporaryDir directory;
	const QString path = WriteIniFile(directory, "");
	Configuration configuration = Configuration::Load(path);
	AlarmSettings alarm;
	alarm.isEnabled = false;
	alarm.days.at(1).time = QTime(5, 5);
	alarm.days.at(5).isActive = true;
	alarm.days.at(5).time = QTime(9, 0);
	configuration.SetAlarm(alarm);

	QVERIFY(configuration.Save(path));
	const Configuration reloaded = Configuration::Load(path);

	QVERIFY(!reloaded.Alarm().isEnabled);
	QCOMPARE(reloaded.Alarm().days.at(1).time, QTime(5, 5));
	QCOMPARE(reloaded.Alarm().days.at(5).time, QTime(9, 0));
	QCOMPARE(ActiveFlags(reloaded.Alarm()), (Days{true, true, true, true, true, true, false}));
}

QTEST_GUILESS_MAIN(ConfigurationTest)
#include "ConfigurationTest.moc"
