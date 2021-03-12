#include <Windows.h>

#include <QApplication>

#include "player_mainwindow.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WSAData wsaData;
	DWORD ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return -1;
	}

	int argc = 0;
	QApplication app(argc, nullptr);
	QCoreApplication::setApplicationName("CSGO Studio Gui");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);

	MainWindow window;
	window.show();
	return app.exec();
}