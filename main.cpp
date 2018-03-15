#include "FileHunterMW.h"
#include <QtWidgets/QApplication>

#include <QProcess>

int main(int argc, char *argv[])
{
#ifdef QT_IS 
#ifndef _WIN64
	QApplication::setLibraryPaths(QStringList(qgetenv("QT_PLUGIN_IS")));
#else
	QApplication::setLibraryPaths(QStringList(qgetenv("QT_PLUGIN_IS_64")));
#endif
#endif // QT_IS

	QApplication a(argc, argv);
	FileHunterMW w;
	w.show();
	return a.exec();
}
