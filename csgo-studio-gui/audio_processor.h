#pragma once

#include <QTimer>
#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include "network_client.h"

class AudioProcessor : public QObject
{
    Q_OBJECT

public:
    AudioProcessor();
    ~AudioProcessor();

    bool Start(class QIODevice* audioDevice);
    void SetDelay(int32_t delay);

    void Stop();

    void SetEnabledClients(const std::set<uint16_t>& clients) { m_allowedClientIds = clients; }

signals:
    void OnTeamspeakClientConnected(quint16 clientId, QString name);
    void OnTeamspeakClientDisconnected(quint16 clientId);
    void OnClientTalking(quint16 clientId);

    void OnInfo(QString str);
    void OnWarning(QString str);
    void OnError(QString str);

public slots:
    void RequestClientsInfos();

protected:
    void RunAudio(class QIODevice* audioDevice);

    NetworkClient m_client;

    std::atomic_bool m_bRunning;
    std::thread m_processThread;

    int32_t m_delay;
    struct Samples
    {
        uint16_t clientId;
        qint64 timestamp;
        uint32_t size;
        std::unique_ptr<uint8_t[]> data;
    };
    std::mutex m_samplesMutex;
    std::list<Samples> m_timedSamples;

    std::set<uint16_t> m_allowedClientIds;
};