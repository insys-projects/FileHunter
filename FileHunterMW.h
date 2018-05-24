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

	FileHunterCore m_FileHunterCore; // Объект, выполняющий поиск и сравнение файлов

	int m_isStop; // 1 - остановлен поиск

	TRowList m_lnEqualRow;		// Список номеров строк, в которых файлы равны 
	TRowList m_lnNoEqualRow;	// Список номеров строк, в которых файлы разные
	TRowList m_lnNotFoundRow;	// Список номеров строк, в которых файл не найден
	TRowList m_lnLeftRightRow;  // Список номеров строк, в которых файлы отмечены для копирования слева направо
	TRowList m_lnRightLeftRow;	// Список номеров строк, в которых файлы отмечены для копирвоания справа налево

	QMap<int, QPushButton*> m_mpEqualButtons;	// Ключ - признак равенства, значение - указатель на кнопку
	QMap<int, TRowList*>  m_mpRowLists;			// Ключ - признак равенства, значение - список номеров строк 

	QFileSystemWatcher m_FileSystemWatcher;

public:
	FileHunterMW(QWidget *parent = Q_NULLPTR);
	~FileHunterMW();

private:
	// Выбор директории
	void GetExistingDirectory(QLineEdit *pEdit);
	// Выбор файла
	void GetFileName(QLineEdit *pEdit);
	// Очистка таблицы
	void ClearTableItems();
	// Создать item с датой и временем последней модификации файла
	void CreateItemDate(const QString &sFileName, int nRow, int nColumn, int isAlignTop = 0);
	// Создать item с версией файла
	void CreateItemVersion(const QString &sVersion, int nRow, int nColumn, int isAlignTop = 0);
	// Создать item с признаком равенства файлов
	void CreateItemEqual(int isEqual, int nRow);
	// Создать item сравнения
	void CreateItemDiff(int isDiff, int nRow);
	// Показать или спрятать строку
	void ShowHideRow(int nRow);
	void ShowHideRow(int nRow, QPushButton *pButton);
	// Сохранение параметров в реестр
	void WriteParams();
	// Чтение параметров из реестра
	void ReadParams();
	// Обновить иконку файла
	void UpdateSvnIcon(QTableWidgetItem* pItem);

	QIcon GetSvnIcon(int nStatus);

private slots:
	void slotSrcBrowseButton();
	void slotDstBrowseButton();
	// Нажата кнопка поиска
	void slotSearch();
	// Найдены файлы
	void slotFindFileCompleted(TFindFilesRecord rFindFile, int nCurrent, int nTotal);
	// Поиск завершен
	void slotSearchCompleted();
	
	// Контекстное меню
	void slotCustomMenuRequested(QPoint pos);

	// Щелчок по элементу таблицы
	void slotItemClicked(QTableWidgetItem* pItem);

	// Щелчок по кнопке Одинаковые файлы
	void slotEqualClicked();

	// Копирование файлов
	void slotCopy();

	// Открытие файла
	void slotOpenFile();
	// Открыть обозреватель хранилища SVN
	void slotSvnRepoBrowser();
	// Открыть журнал SVN
	void slotSvnLog();
	// Открыть фиксацию SVN
	void slotSvnFix();

	void slotSvnUpdate();

	// Сравнить файлы
	void slotMerge();
	// Файл изменился
	void slotFileChanged(const QString &sPath);
};
