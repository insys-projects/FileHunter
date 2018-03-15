#include <QDirIterator>
#include <QtConcurrent>

#include "FileHunterCore.h"
#include <windows.h>

#define MAX_BUF_SIZE 0x1000000

FileHunterCore::FileHunterCore()
{
	m_isStop = 1;
	
	m_sMask = "*.*";

	m_pData1 = (char*)malloc(MAX_BUF_SIZE);
	m_pData2 = (char*)malloc(MAX_BUF_SIZE);

	// ����������� ���������
	qRegisterMetaType<TFindFilesRecord>("TFindFilesRecord");
}

FileHunterCore::~FileHunterCore()
{
	// ��������� ������
	Stop();

	free(m_pData1);
	free(m_pData2);
}

// ���������� �������� �����
void FileHunterCore::SetSrcDir(const QString &sDir)
{
	m_sSrcDir = sDir;
	m_sSrcFile.clear();
}

// ���������� �������� ����
void FileHunterCore::SetSrcFile(const QString &sFile)
{
	m_sSrcFile = sFile;
	m_sSrcDir.clear();
}

// ���������� ����� ��� ������
void FileHunterCore::SetDstDir(const QString &sDir)
{
	m_sDstDir = sDir;
}

// ����� �����
void FileHunterCore::FindFile()
{
	if(!m_sSrcFile.isEmpty())
		emit findFileCompleted(FindFile(m_sSrcFile), 1, 1);
		
	emit searchCompleted();
}

// ����� ������
void FileHunterCore::FindFiles()
{
	// ��������� ����� ������ ������
	QtConcurrent::run(this, &FileHunterCore::FindFilesTh);
}

// ��������� ������
void FileHunterCore::Stop()
{
	if(m_isStop == 1)
		return;

	m_StopMutex.lock();
	m_isStop = 1;
	
	// ���� ���������� ������
	m_StopWaitCondition.wait(&m_StopMutex);

	m_StopMutex.unlock();
}

// ��������� ������
int FileHunterCore::IsEqual(const QString &sFile1, const QString &sFile2)
{
	QFile file1(sFile1);
	QFile file2(sFile2);
	
	int nSize1 = file1.size();
	int nSize2 = file2.size();
	int nCurSize;
	int isEqual = 1;

	if(nSize1 != nSize2)
		// ������� ������, ����� ������
		return 0;

	if(!file1.open(QIODevice::ReadOnly))
		// ������ �������� �����
		return 0;

	if(!file2.open(QIODevice::ReadOnly))
	{	// ������ �������� �����
		file1.close();
		return 0;
	}

	// ������ ���� �����
	while(file1.read(m_pData1, MAX_BUF_SIZE) > 0)
	{
		nCurSize = file2.read(m_pData2, MAX_BUF_SIZE);

		if(memcmp(m_pData1, m_pData2, nCurSize) != 0)
		{	// ������ ������
			isEqual = 0;
			break;
		}
	}

	// ��������� �������� �����
	file1.close();
	file2.close();

	return isEqual;
}

// ����������� �����
void FileHunterCore::FileCopy(const QString &sSrcPath, const QString &sDstPath)
{
	QFile::remove(sDstPath);
	QFile::copy(sSrcPath, sDstPath);
}

// ���������� �����
void FileHunterCore::SetMask(const QString &sMask)
{
	m_sMask = sMask;
}

// ����� �����
TFindFilesRecord FileHunterCore::FindFile(const QString &sFile)
{
	TFindFilesRecord rFindFiles;
	QFileInfo file_info(sFile);
	QString sFindFile;
	QString sStatus;

	rFindFiles.sSrcFile = sFile;
	rFindFiles.nSrcFileStatus = m_mSrcSvnStatus.value(sFile, SVN_STATUS_NORMAL);
		
	QDirIterator it(m_sDstDir, QStringList() << file_info.fileName(), QDir::NoFilter, QDirIterator::Subdirectories);

	// ���������� ��� ��������� �����
	while(it.hasNext())
	{	
		sFindFile = it.next();
		// ��������� � ������ ��������� ����
		rFindFiles.lsFindFiles << sFindFile;
		rFindFiles.lnFindFilesStatus << m_mDstSvnStatus.value(sFindFile, SVN_STATUS_NORMAL);
		// ��������� � ������ ������� ��������� ������
		rFindFiles.lisEqual << IsEqual(sFile, sFindFile);
	}

	return rFindFiles;
}

// ����� ������ ������
void FileHunterCore::FindFilesTh()
{
	QStringList lsFiles;
	QString sFile;
	int i = 1;

	m_StopMutex.lock();
	m_isStop = 0;
	m_StopMutex.unlock();

	if(m_sSrcDir.isEmpty())
	{
		emit searchCompleted();
		return;
	}

	// �������� svn-�������
	m_mSrcSvnStatus = GetSvnStatus(m_sSrcDir);
	m_mDstSvnStatus = GetSvnStatus(m_sDstDir);

	// ������������ ������ ������, ������� ����� ������
	QDirIterator it(m_sSrcDir, m_sMask.split("|"), QDir::Files | QDir::NoDot | QDir::NoDotDot, QDirIterator::Subdirectories);

	while(it.hasNext())
	{
		lsFiles << it.next();

		if(m_isStop)
			break;
	}

	// ����� ������
	foreach(sFile, lsFiles)
	{
		emit findFileCompleted(FindFile(sFile), i++, lsFiles.size());

		Sleep(100);

		if(m_isStop)
			// ��������� ������
			break;
	}

	// ����� ��������
	emit searchCompleted();

	m_StopWaitCondition.wakeOne();

	m_StopMutex.lock();
	m_isStop = 1;
	m_StopMutex.unlock();
}

QMap<QString, int> FileHunterCore::GetSvnStatus(QString sPath)
{
	QMap<QString, int> mSvnStatus;

	QProcess::execute("cmd.exe /C \"svn st " + sPath + " > FH.tmp\"");

	QFile file("FH.tmp");
	QTextStream strm(&file);

	file.open(QIODevice::ReadOnly);

	QString sLine = strm.readLine();

	QString sKey;
	int nData;
	QStringList list;

	while(!sLine.isEmpty())
	{
		list  = sLine.split(" ");
		list.removeAll("");
		sKey  = list[1];
		sKey.replace("\\", "/");
		
		if(list[0] == "M")
			nData = SVN_STATUS_MODIFIED;
		else if(list[0] == "?")
			nData = SVN_STATUS_UNVERSIONED;
		else if(list[0].isEmpty())
			nData = SVN_STATUS_NORMAL;

		mSvnStatus.insert(sKey, nData);

		sLine = strm.readLine();
	}

	return mSvnStatus;
}