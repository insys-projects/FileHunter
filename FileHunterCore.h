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
	QString		sSrcFile;		// �������� ����
	QStringList lsFindFiles;	// ��������� �����
	int			nSrcFileStatus;
	QList<int>  lnFindFilesStatus;
	QList<int>  lisEqual;		// ������� ���������
} TFindFilesRecord;

typedef QList<TFindFilesRecord> FindFilesList;

class FileHunterCore : public QObject
{
	Q_OBJECT

	QString m_sSrcDir;	// �������� �����
	QString m_sSrcFile;	// �������� ����
	QString m_sDstDir;	// ����� ��� ������

	QString m_sMask; 	// ����� ������ ��� ������

	volatile int m_isStop;

	QMutex m_StopMutex;
	QWaitCondition m_StopWaitCondition;

	char *m_pData1;
	char *m_pData2;

	QMap<QString, int> m_mSrcSvnStatus;
	QMap<QString, int> m_mDstSvnStatus;

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
	// ���������� �����
	void SetMask(const QString &sMask);

private:
	// ����� �����
	TFindFilesRecord FindFile(const QString &sFile);
	// ����� ������ ������
	void FindFilesTh();
	
public:
	QMap<QString, int> GetSvnStatus(QString sPath);

signals:
	// ����� ��������
	void searchCompleted();
	// ����� ����� ��������
	void findFileCompleted(TFindFilesRecord, int, int);
};