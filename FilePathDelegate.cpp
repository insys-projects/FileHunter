#include "FilePathDelegate.h"

FilePathDelegate::FilePathDelegate(QObject *parent, QStyleOptionViewItem::Position decorationPosition)
	: QStyledItemDelegate(parent)
{
	m_decorationPosition = decorationPosition;
}

void FilePathDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
	const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;
	opt.decorationAlignment = Qt::AlignHCenter | Qt::AlignTop;
	opt.decorationPosition = m_decorationPosition;
	QStyledItemDelegate::paint(painter, opt, index);
}