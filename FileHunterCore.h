#pragma once

#include <QObject>
#include <QMetaType>
#include <QMutex>
#include <QWaitCondition>

typedef struct
{
	QString		sSrcFile;		// Исходный файл
	QStringList lsFindFiles;	// Найденные файлы
	QList<int>  lisEqual;		// Признак равенства
} TFindFilesRecord;

typedef QList<TFindFilesRecord> FindFilesList;

class FileHunterCore : public QObject
{
Q_OBJECT

	QString m_sSrcDir;	// Исходная папка
	QString m_sSrcFile;	// Исходный файл
	QString m_sDstDir;	// Папка для поиска

	volatile int m_isStop;

	QMutex m_StopMutex;
	QWaitCondition m_StopWaitCondition;

	char *m_pData1;
	char *m_pData2;

public:
	FileHunterCore();
	~FileHunterCore();

	// Установить исходную папку
	void SetSrcDir(const QString &sDir);
	// Установить исходный файл
	void SetSrcFile(const QString &sFile);
	// Установить папку для поиска
	void SetDstDir(const QString &sDir);
	// Поиск файла
	void FindFile();
	// Поиск файлов
	void FindFiles();
	// Остановка поиска
	void Stop();
	// Сравнение файлов
	int IsEqual(const QString &sFile1, const QString &sFile2);
	// Копирование файла
	void FileCopy(const QString &sSrcPath, const QString &sDstPath);

private:
	// Поиск файла
	TFindFilesRecord FindFile(const QString &sFile);
	// Поток поиска файлов
	void FindFilesTh();

signals:
	// Поиск завершен
	void searchCompleted();
	// Поиск файла завершен
	void findFileCompleted(TFindFilesRecord, int, int);
};