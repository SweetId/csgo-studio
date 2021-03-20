#include <Windows.h>

#include <QApplication>

#include "director_mainwindow.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argc = 0;
	QApplication app(argc, nullptr);
	QCoreApplication::setApplicationName("CSGO Studio Director");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);

	MainWindow window;
	window.show();
	return app.exec();
}