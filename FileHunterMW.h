#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FileHunterMW.h"

#include "FileHunterCore.h"

enum
{
	SRC_NAME_COLUMN = 0,
	SRC_DATE_COLUMN,
	EQUAL_COLUMN,
	DST_DATE_COLUMN,
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
	// ������� item � ��������� ��������� ������
	void CreateItemEqual(int isEqual, int nRow);
	// �������� ��� �������� ������
	void ShowHideRow(int nRow);
	void ShowHideRow(int nRow, QPushButton *pButton);
	// ���������� ���������� � ������
	void WriteParams();
	// ������ ���������� �� �������
	void ReadParams();

private slots:
	void slotSrcBrowseButton();
	void slotDstBrowseButton();
	// ������ ������ ������
	void slotSearch();
	// ������� �����
	void slotFindFileCompleted(TFindFilesRecord rFindFile, int nCurrent, int nTotal);
	// ����� ��������
	void slotSearchCompleted();
	
	// ������ �� �������� �������
	void slotItemClicked(QTableWidgetItem* pItem);

	// ������ �� ������ ���������� �����
	void slotEqualClicked();

	// ����������� ������
	void slotCopy();
};
