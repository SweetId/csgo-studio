#pragma once

#include <QWidget>

class QSoundBar : public QWidget
{
    Q_OBJECT

public:
    QSoundBar(QWidget* parent = 0);

protected:
    void paintEvent(QPaintEvent* event);

public slots:
    void setLevel(qreal value);

private:
    qreal m_level;
    QPixmap m_pixmap;
};