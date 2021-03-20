#include <Windows.h>

#include <QApplication>

#include "player_mainwindow.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argc = 0;
	QApplication app(argc, nullptr);
	QCoreApplication::setApplicationName("CSGO Studio Gui");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);

	MainWindow window;
	window.show();
	return app.exec();
}