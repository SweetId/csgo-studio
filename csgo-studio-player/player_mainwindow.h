#include <QMainWindow>
#include <QCamera>
#include <QThread>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

public slots:
    void OnCameraOn(QString camera);
    void OnCameraOff();
    
    void OnMicrophoneOn(QString microphone);
    void OnMicrophoneOff();
    //void OnMicrophoneToggled(bool bToggled);

private:
    class QAudioRecorder* m_microphone;
    class QCamera* m_camera;
    class QCameraThread* m_cameraThread;
    class QCameraViewfinder* m_cameraWidget;
};