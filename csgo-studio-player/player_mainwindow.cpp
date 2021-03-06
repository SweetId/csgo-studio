#include "player_mainwindow.h"

#include "audio_device.h"
#include "qnet_data.h"
#include "sound_bar.h"
#include "ffmpeg/audio_stream_decoder.h"
#include "ffmpeg/audio_stream_encoder.h"
#include "ffmpeg/video_stream_encoder.h"

#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QToolbar>
#include <QToolButton>
#include <QCameraViewfinder>
#include <fstream>

MainWindow::MainWindow()
	: m_microphone(nullptr)
	, m_inputDevice(nullptr)
	, m_camera(nullptr)
	, m_cameraWidget(nullptr)
	, m_videoCapture(nullptr)
	, m_audioEncoder(nullptr)
	, m_videoEncoder(nullptr)
	, m_audioOutput(nullptr)
	, m_audioDevice(nullptr)
	, m_decoder(nullptr)
	, m_bMicrophoneMuted(true)
	, m_mindB(0.f)
{
	QAudioFormat format;
	format.setSampleRate(44100);
	format.setChannelCount(2);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	m_inputDevice = new AudioDevice(format, this);
	connect(m_inputDevice, &AudioDevice::dataAvailable, this, &MainWindow::OnMicrophoneSample);

	QToolBar* toolbar = new QToolBar(this);
	toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
	toolbar->setAllowedAreas(Qt::ToolBarArea::AllToolBarAreas);
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);

	// Network connection
	toolbar->addWidget(new QLabel("Nickname:", toolbar));
	QLineEdit* nicknameText = new QLineEdit("nickname", toolbar);
	toolbar->addWidget(nicknameText);
	toolbar->addWidget(new QLabel("Server IP:", toolbar));
	QLineEdit* serveripText = new QLineEdit("127.0.0.1", toolbar);
	toolbar->addWidget(serveripText);

	QAction* networkOnAction = new QAction(QIcon(":/icons/network_off.png"), tr("Connect to server"), this);
	QAction* networkOffAction = new QAction(QIcon(":/icons/network_on.png"), tr("Disconnect from server"), this);
	QToolButton* networkButton = new QToolButton(toolbar);
	networkButton->setDefaultAction(networkOnAction);
	toolbar->addWidget(networkButton);
	connect(this, &MainWindow::ConnectingToServer, [networkButton, nicknameText, serveripText]() {
		networkButton->setEnabled(false);
		nicknameText->setEnabled(false); 
		serveripText->setEnabled(false);
	});

	connect(networkOnAction, &QAction::triggered, [this, nicknameText, serveripText](bool bTriggered) { ConnectToServer(serveripText->text(), nicknameText->text()); });
	connect(networkOffAction, &QAction::triggered, this, &MainWindow::DisconnectFromServer);

	connect(this, &MainWindow::ConnectedToServer, [networkButton, networkOffAction]() {
		networkButton->setDefaultAction(networkOffAction);
		networkButton->setEnabled(true);
	});
	connect(this, &MainWindow::DisconnectedFromServer, [networkButton, networkOnAction, nicknameText, serveripText]() {
		networkButton->setDefaultAction(networkOnAction);
		nicknameText->setEnabled(true);
		serveripText->setEnabled(true);
	});
	connect(this, &MainWindow::ConnectedToServer, [this, nicknameText]() {
		SendIdentifier(nicknameText->text());
	});

	connect(&m_connection, &QNetClient::ConnectedToServer, this, &MainWindow::ConnectedToServer);
	connect(&m_connection, &QNetClient::DisconnectedFromServer, this, &MainWindow::DisconnectedFromServer);
	connect(&m_connection, &QNetClient::ErrorOccured, this, &MainWindow::OnSocketErrorOccurred);

	// Microphone list and button
	toolbar->addSeparator();
	QComboBox* microphonesList = new QComboBox(toolbar);
	{
		QSet<QString> set;
		set.insert(QAudioDeviceInfo::defaultInputDevice().deviceName());
		microphonesList->addItem(QAudioDeviceInfo::defaultInputDevice().deviceName(), QVariant::fromValue(QAudioDeviceInfo::defaultInputDevice()));
		for (const QAudioDeviceInfo& microphone : QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
		{
			if (!set.contains(microphone.deviceName()))
			{
				set.insert(microphone.deviceName());
				microphonesList->addItem(microphone.deviceName(), QVariant::fromValue(microphone));
			}
		}

		OnMicrophoneChanged(QAudioDeviceInfo::defaultInputDevice());
	}

	//qDebug() << "DEFAULT:" << QAudioDeviceInfo::defaultInputDevice().deviceName()<< QAudioDeviceInfo::defaultInputDevice().isNull();
	toolbar->addWidget(microphonesList);
	connect(microphonesList, &QComboBox::currentTextChanged, [this, microphonesList, format](const QString&) {
		QAudioDeviceInfo data = microphonesList->currentData().value<QAudioDeviceInfo>();
		OnMicrophoneChanged(data);
	});

	QAction* microphoneOnAction = new QAction(QIcon(":/icons/microphone_off.png"), tr("Turn mic on"), this);
	QAction* microphoneOffAction = new QAction(QIcon(":/icons/microphone_on.png"), tr("Turn mic off"), this);
	QToolButton* microphoneButton = new QToolButton(toolbar);

	microphoneButton->setDefaultAction(microphoneOnAction);
	toolbar->addWidget(microphoneButton);
	connect(microphoneOnAction, &QAction::triggered, [microphoneButton, microphoneOffAction, microphonesList]() { microphoneButton->setDefaultAction(microphoneOffAction); microphonesList->setEnabled(false); });
	connect(microphoneOffAction, &QAction::triggered, [microphoneButton, microphoneOnAction, microphonesList]() { microphoneButton->setDefaultAction(microphoneOnAction); microphonesList->setEnabled(true); });

	connect(microphoneOnAction, &QAction::triggered, [this]() { m_bMicrophoneMuted = false; });
	connect(microphoneOffAction, &QAction::triggered, [this]() { m_bMicrophoneMuted = true; });

	// Camera list and button
	toolbar->addSeparator();
	QComboBox* cameraList = new QComboBox(toolbar);
	for (const QCameraInfo& camera : QCameraInfo::availableCameras())
	{
		qDebug() << camera.description();
		cameraList->addItem(camera.description());
	}
	toolbar->addWidget(cameraList);

	QAction* cameraOnAction = new QAction(QIcon(":/icons/camera_off.png"), tr("Turn cam on"), this);
	QAction* cameraOffAction = new QAction(QIcon(":/icons/camera_on.png"), tr("Turn cam off"), this);
	QToolButton* cameraButton = new QToolButton(toolbar);

	cameraButton->setDefaultAction(cameraOnAction);
	toolbar->addWidget(cameraButton);
	connect(cameraOnAction, &QAction::triggered, [cameraButton, cameraOffAction, cameraList]() { cameraButton->setDefaultAction(cameraOffAction); cameraList->setEnabled(false); });
	connect(cameraOffAction, &QAction::triggered, [cameraButton, cameraOnAction, cameraList]() { cameraButton->setDefaultAction(cameraOnAction); cameraList->setEnabled(true); });

	connect(cameraOnAction, &QAction::triggered, [this, cameraList]() { OnCameraOn(cameraList->currentText()); });
	connect(cameraOffAction, &QAction::triggered, this, &MainWindow::OnCameraOff);

	m_cameraWidget = new QCameraViewfinder(this);
	setCentralWidget(m_cameraWidget);
	m_cameraWidget->setFixedSize(640, 480);

	QSoundBar* soundbar = new QSoundBar(this);
	//setCentralWidget(soundbar);
	connect(m_inputDevice, &AudioDevice::levelUpdated, soundbar, &QSoundBar::setLevel);


	AudioSampleDescriptor audioDescriptor(ECodec::Mp2, 2, 64000, 44100, ESampleFormat::S16);
	m_audioEncoder = new AudioStreamEncoder(audioDescriptor);

	VideoSampleDescriptor videoDescriptor(ECodec::h264, 400000, 640, 480, 30, QVideoFrame::PixelFormat::Format_YUV420P);
	m_videoEncoder = new VideoStreamEncoder(videoDescriptor);

	// Local audio playback
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
	m_audioOutput = new QAudioOutput(info, format, this);
	m_audioDevice = m_audioOutput->start();

	m_decoder = new AudioStreamDecoder(audioDescriptor);
}

MainWindow::~MainWindow()
{
	m_microphone->stop();
}

void MainWindow::OnCameraOn(const QString& camera)
{
	for (const QCameraInfo& desc : QCameraInfo::availableCameras())
	{
		if (desc.description() == camera)
		{
			m_camera = new QCamera(desc, this);
			m_camera->setViewfinder(m_cameraWidget);
			
			QCameraViewfinderSettings vfSettings;
			vfSettings.setResolution(640, 480);
			vfSettings.setPixelFormat(QVideoFrame::PixelFormat::Format_YUYV);
			m_camera->setViewfinderSettings(vfSettings);
			m_camera->start();

			auto viewfinderSettings = m_camera->supportedViewfinderSettings();
			for (auto i : viewfinderSettings)
			{
				qDebug() << i.resolution() << i.pixelFormat() << i.maximumFrameRate();
			}

			m_videoCapture = new QCameraImageCapture(m_camera, this);
			m_videoCapture->setCaptureDestination(QCameraImageCapture::CaptureDestination::CaptureToBuffer);
			//m_capture->setBufferFormat(QVideoFrame::PixelFormat::Format_BGR24);
			
			auto res = m_videoCapture->supportedResolutions();
			auto codecs = m_videoCapture->supportedImageCodecs();
			auto formats = m_videoCapture->supportedBufferFormats();
			//m_videoCapture->setBufferFormat(QVideoFrame::PixelFormat::Format_YUV420P);
			auto fmt = m_videoCapture->bufferFormat();
			
			connect(m_videoCapture, &QCameraImageCapture::imageAvailable, this, &MainWindow::OnCameraFrame);
			m_videoCapture->capture();
			break;
		}
	}
}

void MainWindow::OnCameraOff()
{
	if (nullptr != m_camera)
		m_camera->stop();
	delete m_camera;
}

void MainWindow::OnCameraFrame(int id, const QVideoFrame& frame)
{
	QImage image = frame.image().scaled(640, 480);

	QByteArray im((const char*)image.bits(), image.sizeInBytes());
	QByteArray outBuffer;
	m_videoEncoder->Encode(im, outBuffer);
	
	/*if (m_connection.IsConnected())
	{
		QNetCameraFrame header;
		header.id = 0;
		header.size = image.sizeInBytes();
		header.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
		m_connection.Send(TRequestWithData<QNetCameraFrame, QImage>(header, image));
	}*/
	m_videoCapture->capture();
}

void MainWindow::OnMicrophoneSample(const char* data, qint64 len)
{
	QByteArray stereoSound(data, len);
	QByteArray outBuffer;
	m_audioEncoder->Encode(stereoSound, outBuffer);

	if (!m_bMicrophoneMuted && m_connection.IsConnected() && PassNoiseGate(m_inputDevice->level()))
	{
		QNetSoundwave header;
		header.id = 0;
		header.size = outBuffer.size();
		header.timestamp = QDateTime::currentMSecsSinceEpoch();
		m_connection.Send(TRequestWithData<QNetSoundwave, QByteArray>(header, outBuffer));
	}

	if (nullptr != m_decoder)
	{
		QByteArray decoded;
		m_decoder->Decode(outBuffer, decoded);
		m_audioDevice->write(decoded);
	}
}

void MainWindow::ConnectToServer(const QString& serverip, const QString& nickname)
{
	if (serverip.trimmed().isEmpty())
	{
		QMessageBox::warning(this, "Error", "Please enter valid ip");
		return;
	}
	if (nickname.trimmed().isEmpty())
	{
		QMessageBox::warning(this, "Error", "Please enter a nickname before connecting");
		return;
	}

	QStringList server = serverip.split(':');
	QString ip;
	qint16 port = (quint16)37016;
	if (server.size() == 1)
	{
		ip = server[0];
	}
	else if (server.size() == 2)
	{
		ip = server[0];
		port = server[1].toInt();
	}

	m_connection.Start(QHostAddress(ip), port);
}
void MainWindow::DisconnectFromServer()
{
	m_connection.Stop();
}

void MainWindow::OnSocketErrorOccurred(QString message)
{
	QMessageBox::critical(this, "Network Error", message);
	qDebug() << message;
	emit DisconnectFromServer();
}

void MainWindow::SendIdentifier(const QString& nickname)
{
	QNetClientIdentifier header;
	header.name = nickname;
	m_connection.Send(TRequest<QNetClientIdentifier>(header));
}

bool MainWindow::PassNoiseGate(float volumedB)
{
	return volumedB >= 0.1;
}

void MainWindow::OnMicrophoneChanged(const QAudioDeviceInfo& info)
{
	QAudioFormat format;
	format.setSampleRate(44100);
	format.setChannelCount(2);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	m_inputDevice->stop();
	if (nullptr != m_microphone)
	{
		m_microphone->stop();
		m_microphone->disconnect(this);
		delete m_microphone;
	}

	m_microphone = new QAudioInput(info, format, this);
	m_inputDevice->start();
	m_microphone->start(m_inputDevice);
	m_microphone->setVolume(1);
}