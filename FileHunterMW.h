#pragma once

#include <QtWidgets/QMainWindow>
#include <QFileSystemWatcher>

#include "ui_FileHunterMW.h"

#include "FileHunterCore.h"

enum
{
	SRC_NAME_COLUMN = 0,
	SRC_VERSION_COLUMN,
	SRC_DATE_COLUMN,
	EQUAL_COLUMN,
	DIFF_COLUMN,
	DST_DATE_COLUMN,
	DST_VERSION_COLUMN,
	DST_NAME_COLUMN
};

typedef QList<int> TRowList;

class FileHunterMW : public QMainWindow
{
	Q_OBJECT

private:
	Ui::FileHunterMWClass ui;

	FileHunterCore m_FileHunterCore; // ������, ����������� ����� � ��������� ������

	int m_isStop; // 1 - ���������� �����

	TRowList m_lnEqualRow;		// ������ ������� �����, � ������� ����� ����� 
	TRowList m_lnNoEqualRow;	// ������ ������� �����, � ������� ����� ������
	TRowList m_lnNotFoundRow;	// ������ ������� �����, � ������� ���� �� ������
	TRowList m_lnLeftRightRow;  // ������ ������� �����, � ������� ����� �������� ��� ����������� ����� �������
	TRowList m_lnRightLeftRow;	// ������ ������� �����, � ������� ����� �������� ��� ����������� ������ ������

	QMap<int, QPushButton*> m_mpEqualButtons;	// ���� - ������� ���������, �������� - ��������� �� ������
	QMap<int, TRowList*>  m_mpRowLists;			// ���� - ������� ���������, �������� - ������ ������� ����� 

	QFileSystemWatcher m_FileSystemWatcher;

public:
	FileHunterMW(QWidget *parent = Q_NULLPTR);
	~FileHunterMW();

private:
	// ����� ����������
	void GetExistingDirectory(QLineEdit *pEdit);
	// ����� �����
	void GetFileName(QLineEdit *pEdit);
	// ������� �������
	void ClearTableItems();
	// ������� item � ����� � �������� ��������� ����������� �����
	void CreateItemDate(const QString &sFileName, int nRow, int nColumn, int isAlignTop = 0);
	// ������� item � ������� �����
	void CreateItemVersion(const QString &sVersion, int nRow, int nColumn, int isAlignTop = 0);
	// ������� item � ��������� ��������� ������
	void CreateItemEqual(int isEqual, int nRow);
	// ������� item ���������
	void CreateItemDiff(int isDiff, int nRow);
	// �������� ��� �������� ������
	void ShowHideRow(int nRow);
	void ShowHideRow(int nRow, QPushButton *pButton);
	// ���������� ���������� � ������
	void WriteParams();
	// ������ ���������� �� �������
	void ReadParams();
	// �������� ������ �����
	void UpdateSvnIcon(QTableWidgetItem* pItem);

	QIcon GetSvnIcon(int nStatus);

private slots:
	void slotSrcBrowseButton();
	void slotDstBrowseButton();
	// ������ ������ ������
	void slotSearch();
	// ������� �����
	void slotFindFileCompleted(TFindFilesRecord rFindFile, int nCurrent, int nTotal);
	// ����� ��������
	void slotSearchCompleted();
	
	// ����������� ����
	void slotCustomMenuRequested(QPoint pos);

	// ������ �� �������� �������
	void slotItemClicked(QTableWidgetItem* pItem);

	// ������ �� ������ ���������� �����
	void slotEqualClicked();

	// ����������� ������
	void slotCopy();

	// �������� �����
	void slotOpenFile();
	// ������� ������������ ��������� SVN
	void slotSvnRepoBrowser();
	// ������� ������ SVN
	void slotSvnLog();
	// ������� �������� SVN
	void slotSvnFix();

	void slotSvnUpdate();

	// �������� �����
	void slotMerge();
	// ���� ���������
	void slotFileChanged(const QString &sPath);
};
