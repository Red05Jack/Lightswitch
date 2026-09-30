#include "AppController.h"
#include "Configuration.h"
#include "LogConfiguration.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[]) {
	QGuiApplication application(argc, argv);
	QGuiApplication::setApplicationName(QStringLiteral("Lightswitch"));

	const QString defaultConfigPath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("lightswitch.ini"));

	QCommandLineParser parser;
	parser.addHelpOption();
	const QCommandLineOption fullscreenOption(QStringLiteral("fullscreen"), QStringLiteral("Show the window in fullscreen mode."));
	const QCommandLineOption configOption(QStringLiteral("config"), QStringLiteral("Path to the configuration file."), QStringLiteral("file"), defaultConfigPath);
	parser.addOption(fullscreenOption);
	parser.addOption(configOption);
	parser.process(application);

	LogConfiguration::Apply();

	const Configuration configuration = Configuration::Load(parser.value(configOption));
	AppController controller(configuration, parser.isSet(fullscreenOption));

	application.installEventFilter(&controller.InputMonitor());

	QQmlApplicationEngine engine;
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &application,
		[]() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
	engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
	engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Lightswitch/Ui/Main.qml")));

	controller.Start();
	return application.exec();
}
