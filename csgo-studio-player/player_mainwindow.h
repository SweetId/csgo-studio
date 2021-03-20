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
    void OnMicrophoneSample(QAudioBuffer buffer);

    void OnSocketErrorOccurred(QString message);

private:
    void SendIdentifier(const QString& nickname);
    bool PassNoiseGame(float volumedB);

    class QAudioRecorder* m_microphone;
    class QAudioProbe* m_soundCapture;
    class QCamera* m_camera;
    class QCameraViewfinder* m_cameraWidget;
    class QCameraImageCapture* m_videoCapture;

    QNetClient m_connection;

    bool m_bMicrophoneMuted;
    float m_mindB;
};