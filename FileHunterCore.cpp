#include <QDirIterator>
#include <QtConcurrent>

#include "FileHunterCore.h"
#include <windows.h>

#define MAX_BUF_SIZE 0x1000000

FileHunterCore::FileHunterCore()
{
	m_isStop = 1;
	
	m_sMask = "*.*";

	m_isFindFileVersion = false;

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

// �������� ����� ������ �����
void FileHunterCore::SetFindFileVersionEnabled(bool isEnabled)
{
	m_isFindFileVersion = isEnabled;
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

	if(m_isFindFileVersion)
		FindFilesVersions(&rFindFiles);

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

// ����� ������ �����
QString FileHunterCore::FindFileVersion(const QString &sFile)
{
	QString sVersion;
	QFile file(sFile);
	QTextStream strm(&file);
	QString sData;
	int i, idx;

	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return sVersion;

	// ����� ������ � 30 ������ �������
		for(i = 0; i < 30; i++)
	{	
		sData = strm.readLine();
		
		idx = sData.indexOf("Version");

		if(idx != -1)
			break;
	}
		
	if((idx != -1) && (sData.indexOf(":") != -1))
	{
		sData.remove(0, sData.indexOf(":") + 1);
		sVersion = sData.trimmed();
	}
	
	file.close();

	return sVersion;
}

// ����� ������ ������
void FileHunterCore::FindFilesVersions(TFindFilesRecord *prFindFiles)
{
	prFindFiles->sSrcVersion = FindFileVersion(prFindFiles->sSrcFile);

	foreach(QString sFile, prFindFiles->lsFindFiles)
		prFindFiles->lsFindFilesVersions << FindFileVersion(sFile);
}

QMap<QString, int> FileHunterCore::GetSvnStatus(QString sPath)
{
	QMap<QString, int> mSvnStatus;
	QDir dir;
	QString sFHPath = qgetenv("SystemDrive") + "/Users/" + qgetenv("USERNAME") + "/AppData/Roaming/Instrumental Systems/FileHunter";

	if(!dir.exists(sFHPath))
		dir.mkpath(sFHPath);
	
	QProcess::execute("cmd.exe /C svn st " + sPath + " > \"" + sFHPath + "/FH.tmp\"");

	QFile file(sFHPath + "/FH.tmp");
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