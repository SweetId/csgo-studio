#include <QCoreApplication>
#include <QCommandLineParser>

#include "server.h"

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName("CSGO Studio Server");
	QCoreApplication::setApplicationVersion("0.1.0");

	QCommandLineParser parser;
	parser.setApplicationDescription("CSGO Studio Server help");
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption clientsPortOption("clientsport", "Port to bind clients control channel on");
	parser.addOption(clientsPortOption);

	QCommandLineOption directorPortOption("directorport", "Port to bind director control channel on");
	parser.addOption(directorPortOption);

	parser.process(app);

	qint16 clientsPort = 37016;
	if (parser.isSet(clientsPortOption))
		clientsPort = parser.value(clientsPortOption).toInt();
	qint16 directorPort = 37026;
	if (parser.isSet(directorPortOption))
		directorPort = parser.value(directorPortOption).toInt();

	Server server(clientsPort, directorPort);
	return app.exec();
}