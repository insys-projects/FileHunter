#include "EqualDelegate.h"
#include <QModelIndex>
#include <QPainter>
#include <QPoint>


void EqualDelegate::paint(QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
	QImage img;

	int nState = index.data().toInt() & 0xFF;

	switch(nState)
	{
		case YES_EQUAL:
			img.load(":/FileHunterMW/Resources/equal.png");
			break;

		case NO_EQUAL:
			img.load(":/FileHunterMW/Resources/no_equal.png");
			break;

		case LEFT_RIGHT:
			img.load(":/FileHunterMW/Resources/left_right.png");
			break;

		case RIGHT_LEFT:
			img.load(":/FileHunterMW/Resources/right_left.png");
	}
	
	QRect rect = option.rect;

	rect.setLeft(rect.left() + rect.width() / 2 - 9);
	rect.setTop(rect.top() + rect.height() / 2 - 9);
	rect.setSize(QSize(18, 18));

	painter->drawImage(rect, img);	
}


