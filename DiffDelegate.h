#ifndef DIFFDELEGATE_H
#define DIFFDELEGATE_H

#include <QStyledItemDelegate>

class QModelIndex;
class QPainter;
class QStyleOptionViewItem;

class DiffDelegate: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit DiffDelegate(QObject *parent = 0)
		: QStyledItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index) const;
};

#endif // DIFFDELEGATE_H