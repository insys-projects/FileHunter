#ifndef EQUALDELEGATE_H
#define EQUALDELEGATE_H

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
