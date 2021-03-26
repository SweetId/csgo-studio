#pragma once

#include <QMainWindow>

#include "qnet_client.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

signals:
    void ConnectingToServer();
    void ConnectedToServer();
    void DisconnectedFromServer();

    void InitialTimestampChanged(quint64);

public slots:
    void ConnectToServer(const QString& serverip);
    void DisconnectFromServer();

    void OnSocketErrorOccurred(QString message);

    void OnClientIdentifierReceived(const QNetClientIdentifier& header);
    void OnMicrophoneSamplesReceived(const QNetSoundwave& header, const QByteArray& samples);
    void OnServerSessionReceived(const QNetServerSession& header);

private:
    class QMultiTrack* GetOrCreateMultiTrack(quint32 id);

    QNetClient m_connection;
    QMap<quint32, class QMultiTrack*> m_tracks;

    class QMultiTracksPlayer* m_player;

    class QTreeWidget* m_serverTree;

    class QAudioOutput* m_audioOutput;
    class QIODevice* m_audioDevice;
    class StreamDecoder* m_decoder;

    qint64 m_initialTimestamp;
};