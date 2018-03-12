#include "FileHunterMW.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
#ifndef _WIN64
	QApplication::setLibraryPaths(QStringList(qgetenv("QT_PLUGIN_IS")));
#else
	QApplication::setLibraryPaths(QStringList(qgetenv("QT_PLUGIN_IS_64")));
#endif

	QApplication a(argc, argv);
	FileHunterMW w;
	w.show();
	return a.exec();
}
