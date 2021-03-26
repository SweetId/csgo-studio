#pragma once

#include "common.h"

#include <QAudioFormat>
#include <QIODevice>

class CSGOSTUDIO_API AudioDevice : public QIODevice
{
	Q_OBJECT

public:
    AudioDevice(QAudioFormat format, QObject* parent);
    ~AudioDevice();

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char* data, qint64 len) override;

signals:
    void levelUpdated(qreal level);
    void dataAvailable(const char* data, qint64 len);

private:
    const QAudioFormat m_format;

    quint32 m_maxAmplitude;
    qreal m_level; // 0.0 <= m_level <= 1.0
};