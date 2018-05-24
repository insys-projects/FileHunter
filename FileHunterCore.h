#pragma once

#include <QObject>
#include <QMetaType>
#include <QMutex>
#include <QWaitCondition>
#include <QMap>

enum
{
	SVN_STATUS_NORMAL = 0,
	SVN_STATUS_MODIFIED = 1,
	SVN_STATUS_UNVERSIONED = 2
};

typedef struct
{
	QString		sSrcFile;		// Исходный файл
	QStringList lsFindFiles;	// Найденные файлы
	int			nSrcFileStatus;
	QList<int>  lnFindFilesStatus;
	QList<int>  lisEqual;		// Признак равенства
	QString		sSrcVersion;	// Версия исходного файла
	QStringList lsFindFilesVersions; // Версии найденных файлов
} TFindFilesRecord;

typedef QList<TFindFilesRecord> FindFilesList;

class FileHunterCore : public QObject
{
	Q_OBJECT

	QString m_sSrcDir;	// Исходная папка
	QString m_sSrcFile;	// Исходный файл
	QString m_sDstDir;	// Папка для поиска

	QString m_sMask; 	// Маска файлов для поиска

	volatile int m_isStop;

	QMutex m_StopMutex;
	QWaitCondition m_StopWaitCondition;

	char *m_pData1;
	char *m_pData2;

	QMap<QString, int> m_mSrcSvnStatus;
	QMap<QString, int> m_mDstSvnStatus;

	bool m_isFindFileVersion; 

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
	// Установить маску
	void SetMask(const QString &sMask);
	// Включить поиск версии файла
	void SetFindFileVersionEnabled(bool isEnabled);
	// Поиск версии файла
	QString FindFileVersion(const QString &sFile);

private:
	// Поиск файла
	TFindFilesRecord FindFile(const QString &sFile);
	// Поток поиска файлов
	void FindFilesTh();
	// Поиск версий файлов
	void FindFilesVersions(TFindFilesRecord *prFindFiles);
	
public:
	QMap<QString, int> GetSvnStatus(QString sPath);

signals:
	// Поиск завершен
	void searchCompleted();
	// Поиск файла завершен
	void findFileCompleted(TFindFilesRecord, int, int);
};