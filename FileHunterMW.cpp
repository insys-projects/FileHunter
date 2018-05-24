#include "FileHunterMW.h"

#include <QFileDialog>
#include <QDateTime>
#include <QLabel>
#include <QMouseEvent>
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcess>

#include "EqualDelegate.h"
#include "DiffDelegate.h"
#include "FilePathDelegate.h"

FileHunterMW::FileHunterMW(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// Заполнение ассоциативных массивов
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

	// Поиск остановлен
	m_isStop = 1;

	ui.m_pTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.m_pTable->setItemDelegateForColumn(EQUAL_COLUMN, new EqualDelegate(ui.m_pTable));
	ui.m_pTable->setItemDelegateForColumn(DIFF_COLUMN, new DiffDelegate(ui.m_pTable));

	ui.m_pTable->setItemDelegateForColumn(SRC_NAME_COLUMN, new FilePathDelegate(ui.m_pTable, QStyleOptionViewItem::Right));
	ui.m_pTable->setItemDelegateForColumn(DST_NAME_COLUMN, new FilePathDelegate(ui.m_pTable));

	// Контекстное меню
	ui.m_pTable->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.m_pTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomMenuRequested(QPoint)));

	connect(&m_FileHunterCore, SIGNAL(findFileCompleted(TFindFilesRecord, int, int)),
			this, SLOT(slotFindFileCompleted(TFindFilesRecord, int, int)));
	connect(&m_FileHunterCore, SIGNAL(searchCompleted()), this, SLOT(slotSearchCompleted()));

	connect(ui.m_pOpenFileAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenFile()));
	connect(ui.m_pSvnRepobrowserAction, SIGNAL(triggered(bool)), this, SLOT(slotSvnRepoBrowser()));
	connect(ui.m_pSvnLogAction, SIGNAL(triggered(bool)), this, SLOT(slotSvnLog()));
	connect(ui.m_pSvnFixAction, SIGNAL(triggered(bool)), this, SLOT(slotSvnFix()));
	connect(ui.m_pSvnUpdateAction, SIGNAL(triggered(bool)), this, SLOT(slotSvnUpdate()));
	connect(ui.m_pMergeAction, SIGNAL(triggered(bool)), this, SLOT(slotMerge()));

	connect(&m_FileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(slotFileChanged(QString)));

	// Чтение параметров
	ReadParams();
}

FileHunterMW::~FileHunterMW()
{
	// Сохранение параметров
	WriteParams();
}

// Выбор директории
// pEdit - edit для отображения пути к директории
void FileHunterMW::GetExistingDirectory(QLineEdit *pEdit)
{
	QDir dir;
	QString sCurDir = pEdit->text();

	if(!dir.exists(sCurDir))
		sCurDir = ".";

	QString sDir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Выберите папку"),
		sCurDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if(sDir.isEmpty())
		return;

	sDir.replace("/", "\\");

	pEdit->setText(sDir);
}

// Выбор файла
// pEdit - edit для отображения пути к файлу 
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

	QString sFileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Выберите файл"),
													sCurDir, QString::fromLocal8Bit("Все файлы (*.*)"));

	if(sFileName.isEmpty())
		return;

	sFileName.replace("/", "\\");

	pEdit->setText(sFileName);
}

// Очистка таблицы
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

// Создать item с датой и временем последней модификации файла
// sFileName - имя файла
// nRow - номер строки
// nColumn - номер столбца
void FileHunterMW::CreateItemDate(const QString &sFileName, int nRow, int nColumn, int isAlignTop)
{
	// Получаем информацию о файле
	QFileInfo file_info(sFileName);
	// Получаем дату и время последней модификации файла
	QString sDate = file_info.lastModified().toString("dd.MM.yy hh:mm:ss");
	// Создаем item
	QTableWidgetItem *pItem = new QTableWidgetItem(sDate);
	
	if(isAlignTop)
		// Выравнивание текста
		pItem->setTextAlignment(Qt::AlignTop);

	// Устанавливаем item в таблицу
	ui.m_pTable->setItem(nRow, nColumn, pItem);
}

// Создать item с версией файла
void FileHunterMW::CreateItemVersion(const QString &sVersion, int nRow, int nColumn, int isAlignTop)
{
	QTableWidgetItem *pItem = new QTableWidgetItem(sVersion);

	if(isAlignTop)
		pItem->setTextAlignment(Qt::AlignTop);

	ui.m_pTable->setItem(nRow, nColumn, pItem);
}

// Создать item с признаком равенства файлов
void FileHunterMW::CreateItemEqual(int isEqual, int nRow)
{
	QTableWidgetItem *pItem = new QTableWidgetItem();
	
	// Проверяем равенство файлов
	if(isEqual)
	{	// Равно
		pItem->setText(QString::number(YES_EQUAL));
		m_lnEqualRow << nRow;
	}
	else
	{
		pItem->setText(QString::number(NO_EQUAL));
		m_lnNoEqualRow << nRow;
	}
	
	// Устанавливаем item в таблицу
	ui.m_pTable->setItem(nRow, EQUAL_COLUMN, pItem);
}

// Создать item сравнения
void FileHunterMW::CreateItemDiff(int isDiff, int nRow)
{
	QTableWidgetItem *pItem = new QTableWidgetItem();

	pItem->setText(QString::number(isDiff));

	// Устанавливаем item в таблицу
	ui.m_pTable->setItem(nRow, DIFF_COLUMN, pItem);
}

// Показать или спрятать строку
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

// Сохранение параметров в реестр
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

	settings.setValue("SrcFile", ui.m_pSrcDirRadioBtn->isChecked());
	settings.setValue("SrcPath", ui.m_pSrcLineEdit->text());
	settings.setValue("DstPath", ui.m_pDstLineEdit->text());
	settings.setValue("Mask", ui.m_pMaskEdit->text());

	settings.setValue("ShowVersion", ui.m_pShowVersionCheck->isChecked());
}

// Чтение параметров из реестра
void FileHunterMW::ReadParams()
{
	bool isCheck;
	QString str;

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

	isCheck = settings.value("SrcFile", true).toBool();

	if(!isCheck)
		ui.m_pSrcFileRadioBtn->setChecked(true);

	str = settings.value("SrcPath", "").toString();
	ui.m_pSrcLineEdit->setText(str);

	str = settings.value("DstPath", "").toString();
	ui.m_pDstLineEdit->setText(str);

	str = settings.value("Mask", "*.*").toString();
	ui.m_pMaskEdit->setText(str);

	isCheck = settings.value("ShowVersion", true).toBool();
	ui.m_pShowVersionCheck->setChecked(isCheck);
}

// Обновить иконку файла
void FileHunterMW::UpdateSvnIcon(QTableWidgetItem* pItem)
{
	QMap<QString, int> mStatus = m_FileHunterCore.GetSvnStatus(pItem->text());
	int nStatus = mStatus.value(pItem->text(), SVN_STATUS_NORMAL);
			
	pItem->setIcon(GetSvnIcon(nStatus));
}

QIcon FileHunterMW::GetSvnIcon(int nStatus)
{
	QIcon icon;

	switch(nStatus)
	{
		case SVN_STATUS_NORMAL:
			icon.addFile(":/FileHunterMW/Resources/svn_norm.png");
			break;

		case SVN_STATUS_MODIFIED:
			icon.addFile(":/FileHunterMW/Resources/svn_mod.png");
			break;
	}

	return icon;
}

void FileHunterMW::slotSrcBrowseButton()
{
	if(ui.m_pSrcDirRadioBtn->isChecked())
		// Отображение диалога выбора директории
		GetExistingDirectory(ui.m_pSrcLineEdit);
	else
		// Отображение диалога выбора файла
		GetFileName(ui.m_pSrcLineEdit);
}

void FileHunterMW::slotDstBrowseButton()
{
	GetExistingDirectory(ui.m_pDstLineEdit);
}

// Нажата кнопка поиска
void FileHunterMW::slotSearch()
{
	if(ui.m_pSrcLineEdit->text().isEmpty())
		return;

	if(ui.m_pDstLineEdit->text().isEmpty())
		return;

	if(!m_isStop)
	{	// Остановка поиска
		ui.m_pSearchButton->setText(QString::fromLocal8Bit("Поиск"));
		m_FileHunterCore.Stop();
		m_isStop = 1;
		return;
	}

	// Очищаем списки номреов строк
	m_lnEqualRow.clear();
	m_lnNoEqualRow.clear();
	m_lnNotFoundRow.clear();
	m_lnLeftRightRow.clear();
	m_lnRightLeftRow.clear();

	ui.m_pSearchButton->setText(QString::fromLocal8Bit("Прервать"));
	m_isStop = 0;

	// Очищаем таблицу
	ClearTableItems();

	QStringList lsFiles = m_FileSystemWatcher.files();
	m_FileSystemWatcher.removePaths(lsFiles);

	m_FileHunterCore.SetDstDir(ui.m_pDstLineEdit->text());
	m_FileHunterCore.SetMask(ui.m_pMaskEdit->text());
	m_FileHunterCore.SetFindFileVersionEnabled(ui.m_pShowVersionCheck->isChecked());

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

// Найдены файлы
// rFindFile - результат поиска
// nCurrent  - номер текущего файла
// nTotal    - общее количество фалов
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
		ui.m_pTable->setSpan(nRow, SRC_VERSION_COLUMN, nRowSpanCount, 1);
	}

	m_FileSystemWatcher.addPath(rFindFile.sSrcFile);
	m_FileSystemWatcher.addPaths(rFindFile.lsFindFiles);

	// Отображаем исходный файл
	for(i = nRow; i < ui.m_pTable->rowCount(); i++)
	{
		pItem = new QTableWidgetItem(GetSvnIcon(rFindFile.nSrcFileStatus), rFindFile.sSrcFile);

		if(isAlignTop)
			pItem->setTextAlignment(Qt::AlignTop);

		ui.m_pTable->setItem(i, SRC_NAME_COLUMN, pItem);

		if(ui.m_pShowVersionCheck->isChecked())
			// Отображение версии файла
			CreateItemVersion(rFindFile.sSrcVersion, i, SRC_VERSION_COLUMN, isAlignTop);
		
		CreateItemDate(rFindFile.sSrcFile, i, SRC_DATE_COLUMN, isAlignTop);		
	}
	
	// Отображаем найденные файлы
	if(rFindFile.lsFindFiles.isEmpty())
	{
		pItem = new QTableWidgetItem(QString::fromLocal8Bit("Файл не найден"));
		m_lnNotFoundRow << nRow;
		ui.m_pTable->setItem(nRow, DST_NAME_COLUMN, pItem);
		ShowHideRow(nRow);
	}
	else
	{
		for(i = nRow, j = 0; i < ui.m_pTable->rowCount(); i++, j++)
		{
			pItem = new QTableWidgetItem(GetSvnIcon(rFindFile.lnFindFilesStatus[j]), rFindFile.lsFindFiles[j]);
			ui.m_pTable->setItem(i, DST_NAME_COLUMN, pItem);

			if(ui.m_pShowVersionCheck->isChecked())
				// Отображение версии файла
				CreateItemVersion(rFindFile.lsFindFilesVersions[j], i, DST_VERSION_COLUMN);

			CreateItemDate(rFindFile.lsFindFiles[j], i, DST_DATE_COLUMN);
			// Устанавливаем признак равенства
			CreateItemEqual(rFindFile.lisEqual[j], i);
			CreateItemDiff(!rFindFile.lisEqual[j], i);
			ShowHideRow(i);
			ui.m_pTable->update();
		}
	}

	// Прокрутка вниз
	ui.m_pTable->scrollToBottom();

	// Индикация поиска
	QString str;
	
	nPercent = (double)nCurrent / (double)nTotal * 100.;

	str.sprintf("%d/%d   %d%", nCurrent, nTotal, nPercent);

	ui.statusBar->showMessage(QString::fromLocal8Bit("Поиск файлов:   ") + str);
}

// Поиск завершен
void FileHunterMW::slotSearchCompleted()
{
	if(m_isStop == 0)
		slotSearch();
}

// Контекстное меню
void FileHunterMW::slotCustomMenuRequested(QPoint pos)
{
	QTableWidgetItem *pItem = ui.m_pTable->itemAt(pos);
	int nCol;

	if(pItem == 0)
		// Не было щелчка по ячейке таблицы
		return;

	nCol = pItem->column();

	if((nCol == DST_NAME_COLUMN) && (pItem->text() == QString::fromLocal8Bit("Файл не найден")))
		return;

	if((nCol == EQUAL_COLUMN) && (pItem->text().toInt() == NOT_FOUND))
		return;

	QMenu *pMenu    = new QMenu();
	
	switch(nCol)
	{
		case SRC_NAME_COLUMN:
		case DST_NAME_COLUMN:
			pMenu->addAction(ui.m_pOpenFileAction);
			pMenu->addSeparator();
			pMenu->addAction(ui.m_pSvnUpdateAction);
			pMenu->addAction(ui.m_pSvnFixAction);
			pMenu->addAction(ui.m_pSvnLogAction);
			pMenu->addAction(ui.m_pSvnRepobrowserAction);
			break;

		case EQUAL_COLUMN:
			pMenu->addAction(ui.m_pMergeAction);
	}

	pMenu->exec(ui.m_pTable->viewport()->mapToGlobal(pos));

	delete pMenu;
}

// Щелчок по элементу таблицы
void FileHunterMW::slotItemClicked(QTableWidgetItem* pItem)
{
	TRowList *pRowLists;

	int nState = pItem->text().toInt();

	switch(pItem->column())
	{
		case EQUAL_COLUMN:
			break;

		case DIFF_COLUMN:
			if(nState)
				slotMerge();
			return;

		default:
			return;
	}

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

// Щелчок по кнопке Одинаковые файлы
void FileHunterMW::slotEqualClicked()
{
	QPushButton *pButton    = dynamic_cast<QPushButton*>(sender());
	int nEqual = m_mpEqualButtons.key(pButton);
	TRowList  *pEqualList = m_mpRowLists.value(nEqual);
	int nRow;

	foreach(nRow, *pEqualList)
		ShowHideRow(nRow, pButton);
}

// Копирование файлов
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
		sInfo = QString::fromLocal8Bit("Хотите копировать файлы слева направо?");
	}
	else
	{
		pRowList = &m_lnRightLeftRow;
		nState = RIGHT_LEFT;
		nSrcColumn = DST_NAME_COLUMN;
		nDstColumn = SRC_NAME_COLUMN;
		nSrcDateColumn = DST_DATE_COLUMN;
		nDstDateColumn = SRC_DATE_COLUMN;
		sInfo = QString::fromLocal8Bit("Хотите копировать файлы справа налево?");
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

		// Копирование файла
		m_FileHunterCore.FileCopy(sSrcPath, sDstPath);

		pItem = ui.m_pTable->item(nRow, EQUAL_COLUMN);
		pItem->setText(QString::number(YES_EQUAL));

		pItem = ui.m_pTable->item(nRow, nSrcDateColumn);
		sSrcDate = pItem->text();

		pItem = ui.m_pTable->item(nRow, nDstDateColumn);
		pItem->setText(sSrcDate);

		pItem = ui.m_pTable->item(nRow, nDstColumn);
		UpdateSvnIcon(pItem);
	}

	m_lnEqualRow.append(*pRowList);
	qStableSort(m_lnEqualRow);
	pRowList->clear();

	QMessageBox::question(this, "FileHunter", QString::fromLocal8Bit("Копирование завершено."), QMessageBox::Ok);


}

// Открытие файла
void FileHunterMW::slotOpenFile()
{
	QTableWidgetItem *pItem = ui.m_pTable->currentItem();
	QDesktopServices::openUrl(QUrl::fromLocalFile(pItem->text()));
}

// Открыть обозреватель хранилища SVN
void FileHunterMW::slotSvnRepoBrowser()
{
	QTableWidgetItem *pItem = ui.m_pTable->currentItem();
	QProcess::execute("TortoiseProc.exe /command:repobrowser /path:\"" + pItem->text() +"\"");
}

// Открыть журнал SVN
void FileHunterMW::slotSvnLog()
{
	QTableWidgetItem *pItem = ui.m_pTable->currentItem();
	QProcess::execute("TortoiseProc.exe /command:log /path:\"" + pItem->text() + "\"");
}

// Открыть фиксацию SVN
void FileHunterMW::slotSvnFix()
{
	QTableWidgetItem *pItem = ui.m_pTable->currentItem();
	QProcess::execute("TortoiseProc.exe /command:commit /path:\"" + pItem->text() + "\"");
	// Обновить значок статуса svn
	UpdateSvnIcon(pItem);
}

void FileHunterMW::slotSvnUpdate()
{
	QTableWidgetItem *pItem = ui.m_pTable->currentItem();
	QProcess::execute("TortoiseProc.exe /command:update /path:\"" + pItem->text() + "\"");
	// Обновить значок статуса svn
	UpdateSvnIcon(pItem);
}

// Сравнить файлы
void FileHunterMW::slotMerge()
{
	QTableWidgetItem *pItem = ui.m_pTable->currentItem();
	QTableWidgetItem *pSrcItem = ui.m_pTable->item(pItem->row(), SRC_NAME_COLUMN);
	QTableWidgetItem *pDstItem = ui.m_pTable->item(pItem->row(), DST_NAME_COLUMN);

	QProcess::startDetached("TortoiseMerge.exe " + pSrcItem->text() + " " + pDstItem->text());
	
	// Обновить значок статуса svn
	UpdateSvnIcon(pSrcItem);
	UpdateSvnIcon(pDstItem);
}

// Файл изменился
void FileHunterMW::slotFileChanged(const QString &sPath)
{
	QList<QTableWidgetItem*> lpSrcItems = ui.m_pTable->findItems(sPath, Qt::MatchExactly);
	QList<QTableWidgetItem*> lpDstItems;
	QTableWidgetItem *pSrcItem = lpSrcItems[0];
	QTableWidgetItem *pDstItem;
	QTableWidgetItem *pItem;
	QFileInfo file_info(pSrcItem->text());
	int nStatus, nCurStatus;

	// Обновляем иконку svn
	UpdateSvnIcon(pSrcItem);

	// Изменяем дату последней модификации файла
	if(pSrcItem->column() == SRC_NAME_COLUMN)
		pItem = ui.m_pTable->item(pSrcItem->row(), SRC_DATE_COLUMN);
	else
		pItem = ui.m_pTable->item(pSrcItem->row(), DST_DATE_COLUMN);

	if(pItem == 0)
		return;

	pItem->setText(file_info.lastModified().toString("dd.MM.yy hh:mm:ss"));

	// Изменяем версию файла
	if(pSrcItem->column() == SRC_NAME_COLUMN)
		pItem = ui.m_pTable->item(pSrcItem->row(), SRC_VERSION_COLUMN);
	else
		pItem = ui.m_pTable->item(pSrcItem->row(), DST_VERSION_COLUMN);

	pItem->setText(m_FileHunterCore.FindFileVersion(sPath));

	if(pSrcItem->column() == SRC_NAME_COLUMN)
	{
		int i;
		int nRow = pSrcItem->row();
		int nSpan = ui.m_pTable->rowSpan(nRow, SRC_NAME_COLUMN);

		for(i = nRow; i < (nRow + nSpan); i++)
			lpDstItems << ui.m_pTable->item(i, DST_NAME_COLUMN);
	}
	else
		lpDstItems << ui.m_pTable->item(pSrcItem->row(), SRC_NAME_COLUMN);

	foreach(pDstItem, lpDstItems)
	{
		// Проверяем признак равенства
		pItem = ui.m_pTable->item(pDstItem->row(), EQUAL_COLUMN);

		if(m_FileHunterCore.IsEqual(pSrcItem->text(), pDstItem->text()))
			nStatus = YES_EQUAL;
		else
			nStatus = NO_EQUAL;

		nCurStatus = pItem->text().toInt();

		if((nCurStatus >> 8) != 0)
			nStatus = (nStatus << 8) | (nCurStatus & 0xFF);
		else
		{
			TRowList *pRowLists = m_mpRowLists.value(nCurStatus);
			pRowLists->removeOne(pItem->row());

			pRowLists = m_mpRowLists.value(nStatus);
			pRowLists->append(pItem->row());
			qStableSort(*pRowLists);
		}

		pItem->setText(QString::number(nStatus));

		// item сравнения
		pItem = ui.m_pTable->item(pDstItem->row(), DIFF_COLUMN);

		if(nStatus == YES_EQUAL)
			pItem->setText(QString::number(0)); 
		else
			pItem->setText(QString::number(1));
	}
}