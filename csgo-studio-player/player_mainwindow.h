#include <QMainWindow>
#include <QCamera>
#include <QThread>

#include <QTcpSocket>
#include <QUdpSocket>

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
    void OnCameraOn(const QString& camera);
    void OnCameraOff();
   
    void ConnectToServer(const QString& serverip, const QString& nickname);
    void DisconnectFromServer();

private slots:
    void OnCameraFrame(int id, const class QVideoFrame& frame);
    void OnMicrophoneSample(const char* data, qint64 len);

    void OnSocketErrorOccurred(QString message);

private:
    void SendIdentifier(const QString& nickname);
    bool PassNoiseGate(float volumedB);

    void OnMicrophoneChanged(const class QAudioDeviceInfo& info);

    class QAudioInput* m_microphone;
    class AudioDevice* m_inputDevice;

    class QCamera* m_camera;
    class QCameraViewfinder* m_cameraWidget;
    class QCameraImageCapture* m_videoCapture;

    class StreamEncoder* m_encoder;

    // For local debug playback
    class QAudioOutput* m_audioOutput;
    class QIODevice* m_audioDevice;
    class StreamDecoder* m_decoder;

    QNetClient m_connection;

    bool m_bMicrophoneMuted;
    float m_mindB;
};