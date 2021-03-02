#include <QMainWindow>

#include "audio_processor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

private:
    class QAudioOutput* m_output;
    class QIODevice* m_audioDevice;

    void ConnectToTeamspeak();

    AudioProcessor m_processor;
};