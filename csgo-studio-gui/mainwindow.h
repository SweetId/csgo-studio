#include <QMainWindow>
#include <QSet>

#include "audio_processor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

signals:
    void ClientJoinedChannel(quint16 clientId, quint8 channelId);
    void ChannelToggled(QString channelName, bool bToggled);

public slots:
    void OnClientConnected(quint16 clientId, QString name);
    void OnChannelToggled(QString channelName, bool bToggled);

private:
    class QAudioOutput* m_output;
    class QIODevice* m_audioDevice;

    class QTreeWidget* m_serverTree;

    void ConnectToTeamspeak();
    void DisconnectFromTeamspeak();
    void RefreshEnabledChannels();

    AudioProcessor m_processor;

    QSet<QString> m_enabledChannels;
};