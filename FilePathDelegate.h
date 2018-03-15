#ifndef FILEPATHDELEGATE_H
#define FILEPATHDELEGATE_H
/*
Copyright (c) 2009-10 Qtrac Ltd. All rights reserved.

This program or module is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version. It is provided
for educational purposes and is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
the GNU General Public License for more details.
*/

#include <QStyledItemDelegate>
#include <QPainter>


class QModelIndex;
class QPainter;
class QStyleOptionViewItem;

class FilePathDelegate: public QStyledItemDelegate
{
	Q_OBJECT

	QStyleOptionViewItem::Position m_decorationPosition;

public:
	explicit FilePathDelegate(QObject *parent = 0, QStyleOptionViewItem::Position decorationPosition = QStyleOptionViewItem::Left);

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index) const;
};

#endif // FILEPATHDELEGATE_H

