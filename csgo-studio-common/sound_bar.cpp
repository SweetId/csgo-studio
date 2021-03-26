#include "sound_bar.h"

#include <QPainter>

QSoundBar::QSoundBar(QWidget* parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_level = 0;
    setMinimumHeight(30);
    setMinimumWidth(200);
}

void QSoundBar::paintEvent(QPaintEvent* /* event */)
{
    QPainter painter(this);

    painter.setPen(Qt::black);
    painter.drawRect(QRect(painter.viewport().left() + 10,
        painter.viewport().top() + 10,
        painter.viewport().right() - 20,
        painter.viewport().bottom() - 20));
    if (m_level == 0.0)
        return;

    int pos = ((painter.viewport().right() - 20) - (painter.viewport().left() + 11)) * m_level;
    QRect rect(painter.viewport().left() + 11, 
        painter.viewport().top() + 11,
        pos,
        painter.viewport().height() - 22);

   /* QLinearGradient gradient(0,0,rect.width(), 0);
    gradient.setSpread(QGradient::PadSpread);
    gradient.setColorAt(0, Qt::green);
    gradient.setColorAt(0.6, Qt::yellow);
    gradient.setColorAt(0.9, Qt::red);
    painter.fillRect(rect, gradient);*/
    QColor color = Qt::green;
    if (m_level > 0.6)
        color = Qt::yellow;
    if (m_level > 0.85)
        color = Qt::red;

    painter.fillRect(rect, color);
}

void QSoundBar::setLevel(qreal value)
{
    m_level = value;
    update();
}