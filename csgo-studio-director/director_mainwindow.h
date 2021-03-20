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

public slots:
    void ConnectToServer(const QString& serverip);
    void DisconnectFromServer();

    void OnSocketErrorOccurred(QString message);

    void OnClientIdentifierReceived(const QNetClientIdentifier& identifier);
    void OnMicrophoneSamplesReceived(const QNetSoundwave& header, const QByteArray& samples);

private:
    QNetClient m_connection;
    QMap<quint32, class QTrack*> m_tracks;

    class QTreeWidget* m_serverTree;
};