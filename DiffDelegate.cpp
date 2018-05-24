#include "DiffDelegate.h"
#include <QModelIndex>
#include <QPainter>
#include <QPoint>


void DiffDelegate::paint(QPainter *painter,
	const QStyleOptionViewItem &option,
	const QModelIndex &index) const
{
	int isOn = index.data().toInt();
	QString sDiff;
	QFont fn = painter->font();
	QRect rect = option.rect;
	QRect boundingRect;

	fn.setBold(1);

	painter->setFont(fn);

	if(isOn)
		sDiff = "DIFF";

	QFontMetrics fnmetrics(fn);

	boundingRect = fnmetrics.boundingRect(sDiff);

	rect.setLeft(rect.left() + rect.width() / 2 - boundingRect.width() / 2);
	rect.setTop(rect.top() + rect.height() / 2 - boundingRect.height() / 2);
	rect.setSize(QSize(boundingRect.width(), boundingRect.height()));

	painter->setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::FlatCap));
	painter->drawText(rect, 0, sDiff, &boundingRect);
}