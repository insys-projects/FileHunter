#ifndef EQUALDELEGATE_H
#define EQUALDELEGATE_H
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


class QModelIndex;
class QPainter;
class QStyleOptionViewItem;

enum 
{
	NOT_FOUND = 0,
	YES_EQUAL,
	NO_EQUAL,
	LEFT_RIGHT,
	RIGHT_LEFT
};

class EqualDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit EqualDelegate(QObject *parent=0)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

#endif // EQUALDELEGATE_H
