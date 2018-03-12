#pragma once

#include <QObject>
#include <QMetaType>
#include <QMutex>
#include <QWaitCondition>

typedef struct
{
	QString		sSrcFile;		// �������� ����
	QStringList lsFindFiles;	// ��������� �����
	QList<int>  lisEqual;		// ������� ���������
} TFindFilesRecord;

typedef QList<TFindFilesRecord> FindFilesList;

class FileHunterCore : public QObject
{
Q_OBJECT

	QString m_sSrcDir;	// �������� �����
	QString m_sSrcFile;	// �������� ����
	QString m_sDstDir;	// ����� ��� ������

	volatile int m_isStop;

	QMutex m_StopMutex;
	QWaitCondition m_StopWaitCondition;

	char *m_pData1;
	char *m_pData2;

public:
	FileHunterCore();
	~FileHunterCore();

	// ���������� �������� �����
	void SetSrcDir(const QString &sDir);
	// ���������� �������� ����
	void SetSrcFile(const QString &sFile);
	// ���������� ����� ��� ������
	void SetDstDir(const QString &sDir);
	// ����� �����
	void FindFile();
	// ����� ������
	void FindFiles();
	// ��������� ������
	void Stop();
	// ��������� ������
	int IsEqual(const QString &sFile1, const QString &sFile2);
	// ����������� �����
	void FileCopy(const QString &sSrcPath, const QString &sDstPath);

private:
	// ����� �����
	TFindFilesRecord FindFile(const QString &sFile);
	// ����� ������ ������
	void FindFilesTh();

signals:
	// ����� ��������
	void searchCompleted();
	// ����� ����� ��������
	void findFileCompleted(TFindFilesRecord, int, int);
};