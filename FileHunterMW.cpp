#include "FileHunterMW.h"

#include <QFileDialog>
#include <QDateTime>
#include <QLabel>
#include <QMouseEvent>
#include <QSettings>
#include <QMessageBox>

#include "EqualDelegate.h"

FileHunterMW::FileHunterMW(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// ���������� ������������� ��������
	m_mpEqualButtons[YES_EQUAL]  = ui.m_pEqualButton;
	m_mpEqualButtons[NO_EQUAL]   = ui.m_pNoEqualButton;
	m_mpEqualButtons[LEFT_RIGHT] = ui.m_pLeftRightButton;
	m_mpEqualButtons[RIGHT_LEFT] = ui.m_pRightLeftButton;
	m_mpEqualButtons[NOT_FOUND]  = ui.m_pNotFoundButton;

	m_mpRowLists[YES_EQUAL]    = &m_lnEqualRow;
	m_mpRowLists[NO_EQUAL]     = &m_lnNoEqualRow;
	m_mpRowLists[LEFT_RIGHT]   = &m_lnLeftRightRow;
	m_mpRowLists[RIGHT_LEFT]   = &m_lnRightLeftRow;
	m_mpRowLists[NOT_FOUND] = &m_lnNotFoundRow;

	// ����� ����������
	m_isStop = 1;

	ui.m_pTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.m_pTable->setItemDelegateForColumn(EQUAL_COLUMN, new EqualDelegate(ui.m_pTable));

	connect(&m_FileHunterCore, SIGNAL(findFileCompleted(TFindFilesRecord, int, int)),
			this, SLOT(slotFindFileCompleted(TFindFilesRecord, int, int)));
	connect(&m_FileHunterCore, SIGNAL(searchCompleted()), this, SLOT(slotSearchCompleted()));

	// ������ ����������
	ReadParams();
}

FileHunterMW::~FileHunterMW()
{
	// ���������� ����������
	WriteParams();
}

// ����� ����������
// pEdit - edit ��� ����������� ���� � ����������
void FileHunterMW::GetExistingDirectory(QLineEdit *pEdit)
{
	QDir dir;
	QString sCurDir = pEdit->text();

	if(!dir.exists(sCurDir))
		sCurDir = ".";

	QString sDir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("�������� �����"),
		sCurDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if(sDir.isEmpty())
		return;

	sDir.replace("/", "\\");

	pEdit->setText(sDir);
}

// ����� �����
// pEdit - edit ��� ����������� ���� � ����� 
void FileHunterMW::GetFileName(QLineEdit *pEdit)
{
	QFile file;
	QString sCurFile = pEdit->text();
	QString sCurDir;

	file.setFileName(sCurFile);

	if(file.exists())
		sCurDir = QFileInfo(sCurFile).absolutePath();
	else
		sCurDir = ".";

	QString sFileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("�������� ����"),
													sCurDir, QString::fromLocal8Bit("��� ����� (*.*)"));

	if(sFileName.isEmpty())
		return;

	sFileName.replace("/", "\\");

	pEdit->setText(sFileName);
}

// ������� �������
void FileHunterMW::ClearTableItems()
{
	QTableWidgetItem *pItem;
	int i, j;

	for(i = 0; i < ui.m_pTable->rowCount(); i++)
		for(j = 0; j < ui.m_pTable->columnCount(); j++)
		{
			pItem = ui.m_pTable->item(i, j);

			if(pItem)
				delete pItem;
		}

	ui.m_pTable->setRowCount(0);
}

// ������� item � ����� � �������� ��������� ����������� �����
// sFileName - ��� �����
// nRow - ����� ������
// nColumn - ����� �������
void FileHunterMW::CreateItemDate(const QString &sFileName, int nRow, int nColumn, int isAlignTop)
{
	// �������� ���������� � �����
	QFileInfo file_info(sFileName);
	// �������� ���� � ����� ��������� ����������� �����
	QString sDate = file_info.lastModified().toString("dd.MM.yy hh:mm:ss");
	// ������� item
	QTableWidgetItem *pItem = new QTableWidgetItem(sDate);
	
	if(isAlignTop)
		// ������������ ������
		pItem->setTextAlignment(Qt::AlignTop);

	// ������������� item � �������
	ui.m_pTable->setItem(nRow, nColumn, pItem);
}

// ������� item � ��������� ��������� ������
void FileHunterMW::CreateItemEqual(int isEqual, int nRow)
{
	QTableWidgetItem *pItem = new QTableWidgetItem();
	
	// ��������� ��������� ������
	if(isEqual)
	{	// �����
		pItem->setText(QString::number(YES_EQUAL));
		m_lnEqualRow << nRow;
	}
	else
	{
		pItem->setText(QString::number(NO_EQUAL));
		m_lnNoEqualRow << nRow;
	}
	
	// ������������� item � �������
	ui.m_pTable->setItem(nRow, EQUAL_COLUMN, pItem);
}

// �������� ��� �������� ������
void FileHunterMW::ShowHideRow(int nRow)
{
	QTableWidgetItem *pItem = ui.m_pTable->item(nRow, EQUAL_COLUMN);
	int nEqual;

	if(pItem == 0)
		nEqual = NOT_FOUND;
	else
		nEqual = pItem->text().toInt();

	ShowHideRow(nRow, m_mpEqualButtons.value(nEqual));
}

void FileHunterMW::ShowHideRow(int nRow, QPushButton *pButton)
{
	if(pButton->isChecked())
		ui.m_pTable->showRow(nRow);
	else
		ui.m_pTable->hideRow(nRow);
}

// ���������� ���������� � ������
void FileHunterMW::WriteParams()
{
	QSettings settings("Instrumental Systems", "FileHunter");

	settings.beginGroup("Show");
	settings.setValue("Equal",     ui.m_pEqualButton->isChecked());
	settings.setValue("NoEqual",   ui.m_pNoEqualButton->isChecked());
	settings.setValue("LeftRight", ui.m_pLeftRightButton->isChecked());
	settings.setValue("RightLeft", ui.m_pRightLeftButton->isChecked());
	settings.setValue("NotFound",  ui.m_pNotFoundButton->isChecked());
	settings.endGroup();
}

// ������ ���������� �� �������
void FileHunterMW::ReadParams()
{
	bool isCheck;

	QSettings settings("Instrumental Systems", "FileHunter");

	settings.beginGroup("Show");
	isCheck = settings.value("Equal", true).toBool();
	ui.m_pEqualButton->setChecked(isCheck);

	isCheck = settings.value("NoEqual", true).toBool();
	ui.m_pNoEqualButton->setChecked(isCheck);

	isCheck = settings.value("LeftRight", true).toBool();
	ui.m_pLeftRightButton->setChecked(isCheck);

	isCheck = settings.value("RightLeft", true).toBool();
	ui.m_pRightLeftButton->setChecked(isCheck);

	isCheck = settings.value("NotFound", true).toBool();
	ui.m_pNotFoundButton->setChecked(isCheck);
	settings.endGroup();
}

void FileHunterMW::slotSrcBrowseButton()
{
	if(ui.m_pSrcDirRadioBtn->isChecked())
		// ����������� ������� ������ ����������
		GetExistingDirectory(ui.m_pSrcLineEdit);
	else
		// ����������� ������� ������ �����
		GetFileName(ui.m_pSrcLineEdit);
}

void FileHunterMW::slotDstBrowseButton()
{
	GetExistingDirectory(ui.m_pDstLineEdit);
}

// ������ ������ ������
void FileHunterMW::slotSearch()
{
	if(ui.m_pSrcLineEdit->text().isEmpty())
		return;

	if(ui.m_pDstLineEdit->text().isEmpty())
		return;

	if(!m_isStop)
	{	// ��������� ������
		ui.m_pSearchButton->setText(QString::fromLocal8Bit("�����"));
		m_FileHunterCore.Stop();
		m_isStop = 1;
		return;
	}

	// ������� ������ ������� �����
	m_lnEqualRow.clear();
	m_lnNoEqualRow.clear();
	m_lnNotFoundRow.clear();
	m_lnLeftRightRow.clear();
	m_lnRightLeftRow.clear();

	ui.m_pSearchButton->setText(QString::fromLocal8Bit("��������"));
	m_isStop = 0;

	// ������� �������
	ClearTableItems();

	m_FileHunterCore.SetDstDir(ui.m_pDstLineEdit->text());

	if(ui.m_pSrcFileRadioBtn->isChecked())
	{
		m_FileHunterCore.SetSrcFile(ui.m_pSrcLineEdit->text());
		m_FileHunterCore.FindFile();
	}
	else
	{
		m_FileHunterCore.SetSrcDir(ui.m_pSrcLineEdit->text());
		m_FileHunterCore.FindFiles();
	}
}

// ������� �����
// rFindFile - ��������� ������
// nCurrent  - ����� �������� �����
// nTotal    - ����� ���������� �����
void FileHunterMW::slotFindFileCompleted(TFindFilesRecord rFindFile, int nCurrent, int nTotal)
{
	int nColumnCount = ui.m_pTable->columnCount();
	int nRow = ui.m_pTable->rowCount();
	int nColumn = 0;
	int i;
	int j;
	int nRowSpanCount = rFindFile.lsFindFiles.size();
	int nPercent;
	QTableWidgetItem *pItem;
	int isAlignTop = 0;
	
	if(nRowSpanCount == 0)
		nRowSpanCount = 1;

	if(nRowSpanCount > 1)
		isAlignTop = 1;
	
	ui.m_pTable->setRowCount(nRow + nRowSpanCount);

	if(nRowSpanCount > 1)
	{
		ui.m_pTable->setSpan(nRow, SRC_NAME_COLUMN, nRowSpanCount, 1);
		ui.m_pTable->setSpan(nRow, SRC_DATE_COLUMN, nRowSpanCount, 1);
	}

	// ���������� �������� ����
	for(i = nRow; i < ui.m_pTable->rowCount(); i++)
	{
		pItem = new QTableWidgetItem(rFindFile.sSrcFile);

		if(isAlignTop)
			pItem->setTextAlignment(Qt::AlignTop);

		ui.m_pTable->setItem(i, SRC_NAME_COLUMN, pItem);
		CreateItemDate(rFindFile.sSrcFile, i, SRC_DATE_COLUMN, isAlignTop);
	}
	
	// ���������� ��������� �����
	if(rFindFile.lsFindFiles.isEmpty())
	{
		pItem = new QTableWidgetItem(QString::fromLocal8Bit("���� �� ������"));
		m_lnNotFoundRow << nRow;
		ui.m_pTable->setItem(nRow, DST_NAME_COLUMN, pItem);
		ShowHideRow(nRow);
	}
	else
	{
		for(i = nRow, j = 0; i < ui.m_pTable->rowCount(); i++, j++)
		{
			ui.m_pTable->setItem(i, DST_NAME_COLUMN, new QTableWidgetItem(rFindFile.lsFindFiles[j]));
			CreateItemDate(rFindFile.lsFindFiles[j], i, DST_DATE_COLUMN);
			// ������������� ������� ���������
			CreateItemEqual(rFindFile.lisEqual[j], i);
			ShowHideRow(i);
			ui.m_pTable->update();
		}
	}

	// ��������� ����
	ui.m_pTable->scrollToBottom();

	// ��������� ������
	QString str;
	
	nPercent = (double)nCurrent / (double)nTotal * 100.;

	str.sprintf("%d/%d   %d%", nCurrent, nTotal, nPercent);

	ui.statusBar->showMessage(QString::fromLocal8Bit("����� ������:   ") + str);
}

// ����� ��������
void FileHunterMW::slotSearchCompleted()
{
	if(m_isStop == 0)
		slotSearch();
}

// ������ �� �������� �������
void FileHunterMW::slotItemClicked(QTableWidgetItem* pItem)
{
	TRowList *pRowLists;

	int nState = pItem->text().toInt();

	if(pItem->column() != EQUAL_COLUMN)
		return;
	
	switch(nState & 0xFF)
	{
		case YES_EQUAL:
		case NO_EQUAL:
			pRowLists = m_mpRowLists.value(nState);
			pRowLists->removeOne(pItem->row());
			m_lnLeftRightRow.append(pItem->row());
			qStableSort(m_lnLeftRightRow);
			nState = nState << 8;
			nState |= LEFT_RIGHT & 0xFF;
			break;

		case LEFT_RIGHT:
			m_lnLeftRightRow.removeOne(pItem->row());
			m_lnRightLeftRow.append(pItem->row());
			qStableSort(m_lnRightLeftRow);
			nState &= 0xFF00;
			nState |= RIGHT_LEFT & 0xFF;
			break;

		case RIGHT_LEFT:
			m_lnRightLeftRow.removeOne(pItem->row());
			pRowLists = m_mpRowLists.value(nState >> 8);
			pRowLists->append(pItem->row());
			qStableSort(*pRowLists);
			nState >>= 8;
			break;
	}

	pItem->setText(QString::number(nState));
}

// ������ �� ������ ���������� �����
void FileHunterMW::slotEqualClicked()
{
	QPushButton *pButton    = dynamic_cast<QPushButton*>(sender());
	int nEqual = m_mpEqualButtons.key(pButton);
	TRowList  *pEqualList = m_mpRowLists.value(nEqual);
	int nRow;

	foreach(nRow, *pEqualList)
		ShowHideRow(nRow, pButton);
}

// ����������� ������
void FileHunterMW::slotCopy()
{
	TRowList *pRowList;
	int nState;
	QString sSrcPath, sDstPath;
	int nSrcColumn, nDstColumn;
	QString sSrcDate;
	int nSrcDateColumn, nDstDateColumn;
	int nRow;
	QTableWidgetItem *pItem;
	QString sInfo;

	if(sender() == ui.m_pLeftRightCopyButton)
	{
		pRowList = &m_lnLeftRightRow;
		nState = LEFT_RIGHT;
		nSrcColumn = SRC_NAME_COLUMN;
		nDstColumn = DST_NAME_COLUMN;
		nSrcDateColumn = SRC_DATE_COLUMN;
		nDstDateColumn = DST_DATE_COLUMN;
		sInfo = QString::fromLocal8Bit("������ ���������� ����� ����� �������?");
	}
	else
	{
		pRowList = &m_lnRightLeftRow;
		nState = RIGHT_LEFT;
		nSrcColumn = DST_NAME_COLUMN;
		nDstColumn = SRC_NAME_COLUMN;
		nSrcDateColumn = DST_DATE_COLUMN;
		nDstDateColumn = SRC_DATE_COLUMN;
		sInfo = QString::fromLocal8Bit("������ ���������� ����� ������ ������?");
	}

	int nRet = QMessageBox::question(this, "FileHunter", sInfo);

	if(nRet == QMessageBox::No)
		return;

	foreach(nRow, *pRowList)
	{
		pItem = ui.m_pTable->item(nRow, nSrcColumn);
		sSrcPath = pItem->text();

		pItem = ui.m_pTable->item(nRow, nDstColumn);
		sDstPath = pItem->text();

		// ����������� �����
		m_FileHunterCore.FileCopy(sSrcPath, sDstPath);

		pItem = ui.m_pTable->item(nRow, EQUAL_COLUMN);
		pItem->setText(QString::number(YES_EQUAL));

		pItem = ui.m_pTable->item(nRow, nSrcDateColumn);
		sSrcDate = pItem->text();

		pItem = ui.m_pTable->item(nRow, nDstDateColumn);
		pItem->setText(sSrcDate);
	}

	m_lnEqualRow.append(*pRowList);
	qStableSort(m_lnEqualRow);
	pRowList->clear();

	QMessageBox::question(this, "FileHunter", QString::fromLocal8Bit("����������� ���������."), QMessageBox::Ok);
}