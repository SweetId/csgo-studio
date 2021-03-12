#include "player_mainwindow.h"

#include <QAudioFormat>
#include <QAudioRecorder>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QDebug>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QToolbar>
#include <QToolButton>
#include <QCameraViewfinder>

MainWindow::MainWindow()
	: m_camera(nullptr)
	, m_microphone(new QAudioRecorder(this))
{
	QToolBar* toolbar = new QToolBar(this);
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);

	// Microphone list and button
	QComboBox* microphonesList = new QComboBox(toolbar);
	{
		QSet<QString> items;
		for (const QString& microphone : m_microphone->audioInputs())
			items.insert(microphone);
		microphonesList->addItems(items.values());
	}
	toolbar->addWidget(microphonesList);
	QAction* microphoneOnAction = new QAction(QIcon(":/icons/microphone_off.png"), tr("Turn mic on"), this);
	QAction* microphoneOffAction = new QAction(QIcon(":/icons/microphone_on.png"), tr("Turn mic off"), this);
	QToolButton* microphoneButton = new QToolButton(toolbar);

	microphoneButton->setDefaultAction(microphoneOnAction);
	toolbar->addWidget(microphoneButton);
	connect(microphoneOnAction, &QAction::triggered, [microphoneButton, microphoneOffAction, microphonesList]() { microphoneButton->setDefaultAction(microphoneOffAction); microphonesList->setEnabled(false); });
	connect(microphoneOffAction, &QAction::triggered, [microphoneButton, microphoneOnAction, microphonesList]() { microphoneButton->setDefaultAction(microphoneOnAction); microphonesList->setEnabled(false); });

	connect(microphoneOnAction, &QAction::triggered, [this, microphonesList]() { OnMicrophoneOn(microphonesList->currentText()); });
	connect(microphoneOffAction, &QAction::triggered, this, &MainWindow::OnMicrophoneOff);

	// Camera list and button
	QComboBox* cameraList = new QComboBox(toolbar);
	for (const QCameraInfo& camera : QCameraInfo::availableCameras())
		cameraList->addItem(camera.description());
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
}

MainWindow::~MainWindow() {}

void MainWindow::OnCameraOn(QString camera)
{
	for (const QCameraInfo& desc : QCameraInfo::availableCameras())
	{
		if (desc.description() == camera)
		{
			m_camera = new QCamera(desc, this);
			m_camera->setViewfinder(m_cameraWidget);
			m_camera->start();
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
void MainWindow::OnMicrophoneOn(QString microphone)
{
	/*
	QAudioEncoderSettings format;
	// Set up the format, eg.
	format.setSampleRate(48000);
	format.setChannelCount(2);
	format.setCodec("audio/pcm");
	m_microphone->setEncodingSettings(format);*/
	m_microphone->setAudioInput(microphone);
}

void MainWindow::OnMicrophoneOff()
{
	delete m_microphone;
	m_microphone = nullptr;
}