#include "player_mainwindow.h"

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

#include "qnet_data.h"

MainWindow::MainWindow()
	: m_camera(nullptr)
	, m_microphone(new QAudioRecorder(this))
	, m_bMicrophoneMuted(true)
	, m_mindB(0.f)
{
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
		QSet<QString> items;
		for (const QString& microphone : m_microphone->audioInputs())
			items.insert(microphone);
		microphonesList->addItems(items.values());
	}
	toolbar->addWidget(microphonesList);
	connect(microphonesList, &QComboBox::currentTextChanged, [this](const QString& mic) {
		m_microphone->setAudioInput(mic);
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


	QAudioEncoderSettings settings;
	settings.setCodec("audio/pcm");
	settings.setChannelCount(2);
	settings.setSampleRate(48000);
	m_microphone->setEncodingSettings(settings);
	m_microphone->setAudioInput(m_microphone->defaultAudioInput());
	m_microphone->record();

	m_soundCapture = new QAudioProbe(this);
	m_soundCapture->setSource(m_microphone);
	connect(m_soundCapture, &QAudioProbe::audioBufferProbed, this, &MainWindow::OnMicrophoneSample);
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
			m_camera->start();

			m_videoCapture = new QCameraImageCapture(m_camera, this);
			m_videoCapture->setCaptureDestination(QCameraImageCapture::CaptureDestination::CaptureToBuffer);
			//m_capture->setBufferFormat(QVideoFrame::PixelFormat::Format_BGR24);
			
			auto res = m_videoCapture->supportedResolutions();
			auto codecs = m_videoCapture->supportedImageCodecs();
			auto formats = m_videoCapture->supportedBufferFormats();
			//m_capture->setEncodingSettings(settings);
			
			m_videoCapture->capture();
			connect(m_videoCapture, &QCameraImageCapture::imageAvailable, this, &MainWindow::OnCameraFrame);
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
	/*QImage image = frame.image().scaled(640, 480);
	
	if (m_connection.IsConnected())
	{
		QNetCameraFrame header;
		header.id = 0;
		header.size = image.sizeInBytes();
		header.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
		m_connection.Send(TRequestWithData<QNetCameraFrame, QImage>(header, image));
	}*/
	m_videoCapture->capture();
}

float ConvertToStereo(const QAudioBuffer& inBuffer, QByteArray& outBuffer)
{
	QAudioFormat format = inBuffer.format();
	auto rate = format.sampleRate();
	auto size = format.sampleSize();
	auto codec = format.codec();
	auto type = format.sampleType();
	auto channels = format.channelCount();
	auto samples = inBuffer.sampleCount();

	outBuffer.reserve(2 * samples);

	float maxVal = 0;

	switch (type)
	{
	case QAudioFormat::SampleType::SignedInt:
	{
		for (qint32 i = 0; i < samples; ++i)
		{
			// Store as unsigned
			quint16 left = 0;
			quint16 right = 0;

			if (size == 8)
			{
				const QAudioBuffer::S8S* frames = inBuffer.data<QAudioBuffer::S8S>();
				left = quint16((frames[i].left / 127.f) * 32768 + 32768);
				right = quint16((frames[i].right / 127.f) * 32768 + 32768);
			}
			else if (size == 16)
			{
				const QAudioBuffer::S16S* frames = inBuffer.data<QAudioBuffer::S16S>();
				left = frames[i].left + 32768;
				right = frames[i].right + 32768;
			}
			outBuffer.append((char*)&left, sizeof(quint16));
			outBuffer.append((char*)&right, sizeof(quint16));
			maxVal = qMax(maxVal, qAbs(left / 32768.f));
			maxVal = qMax(maxVal, qAbs(right / 32768.f));
		}
	}
	break;
	case QAudioFormat::SampleType::UnSignedInt:
	{
		for (qint32 i = 0; i < samples; ++i)
		{
			quint16 left = 0;
			quint16 right = 0;

			if (size == 8)
			{
				const QAudioBuffer::S8U* frames = inBuffer.data<QAudioBuffer::S8U>();
				left = quint16((frames[i].left / 255.f) * 32768);
				right = quint16((frames[i].right / 255.f) * 32768);
			}
			else if (size == 16)
			{
				const QAudioBuffer::S16U* frames = inBuffer.data<QAudioBuffer::S16U>();
				left = frames[i].left + 32768;
				right = frames[i].right + 32768;
			}
			maxVal = qMax(maxVal, qAbs(left / 32768.f));
			maxVal = qMax(maxVal, qAbs(right / 32768.f));
		}
	}
	break;
	case QAudioFormat::SampleType::Float:
	{
		if (size != 32)
			break;
		const QAudioBuffer::S32F* frames = inBuffer.data<QAudioBuffer::S32F>();
		for (qint32 i = 0; i < samples; ++i)
		{
			quint16 left = quint16(frames[i].left * 32768 + 32768);
			quint16 right = quint16(frames[i].right * 32768 + 32768);
			outBuffer.append((char*)&left, sizeof(quint16));
			outBuffer.append((char*)&right, sizeof(quint16));
			maxVal = qMax(maxVal, qAbs(left / 32768.f));
			maxVal = qMax(maxVal, qAbs(right / 32768.f));
		}
	}
	break;
	}

	return 20 * std::log10(maxVal);
}

void MainWindow::OnMicrophoneSample(QAudioBuffer buffer)
{
	QByteArray stereoSound;
	float volumedB = ConvertToStereo(buffer, stereoSound);

	if (!m_bMicrophoneMuted && m_connection.IsConnected())//&& PassNoiseGame(volumedB))
	{
		QNetSoundwave header;
		header.id = 0;
		header.size = stereoSound.size();
		header.timestamp = QDateTime::currentMSecsSinceEpoch();
		m_connection.Send(TRequestWithData<QNetSoundwave, QByteArray>(header, stereoSound));
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

bool MainWindow::PassNoiseGame(float volumedB)
{
	return volumedB >= m_mindB;
}