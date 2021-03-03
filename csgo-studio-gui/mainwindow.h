#include <QMainWindow>

#include "audio_processor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

signals:
    void OnClientJoinedChannel(quint16 clientId, quint8 channelId);

public slots:
    void OnClientConnected(quint16 clientId, QString name);

private:
    class QAudioOutput* m_output;
    class QIODevice* m_audioDevice;

    void ConnectToTeamspeak();

    AudioProcessor m_processor;
};