#pragma once

#include <QTimer>
#include <atomic>
#include <list>
#include <thread>

#include "tcp_socket.h"

class AudioProcessor
{
public:
    AudioProcessor();
    ~AudioProcessor();

    bool Start(class QIODevice* audioDevice);
    void SetDelay(int32_t delay);

protected:

    void RunClient();
    void RunAudio(class QIODevice* audioDevice);

    TcpSocket m_client;
    std::atomic_bool m_bRunning;
    std::thread m_networkThread;
    std::thread m_processThread;

    int32_t m_delay;
    struct Samples
    {
        qint64 timestamp;
        uint32_t size;
        std::unique_ptr<uint8_t[]> data;
    };
    std::list<Samples> m_timedSamples;
};