#include "mainwindow.h"

#include <QAudioFormat>
#include <QAudioOutput>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolbar>

MainWindow::MainWindow()
	: m_output(nullptr)
{
	QToolBar* toolbar = new QToolBar(this);
	toolbar->addAction("Connect", [this]() { ConnectToTeamspeak(); });
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);

	QSlider* delay = new QSlider(this);
	delay->setOrientation(Qt::Orientation::Horizontal);
	delay->setMinimum(0);
	delay->setMaximum(5 * 60); // 5 min

	QLabel* delayText = new QLabel("Delay: 0s", this);
	connect(delay, &QSlider::valueChanged, [this, delayText](int val) {
		delayText->setText(QString("Delay: %1s").arg(val));
		m_processor.SetDelay(val);
	});

	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(delay);
	layout->addWidget(delayText);

	QWidget* central = new QWidget(this);
	central->setLayout(layout);
	setCentralWidget(central);

	QAudioFormat format;
	// Set up the format, eg.
	format.setSampleRate(48000);
	format.setChannelCount(2);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
	if (!info.isFormatSupported(format))
	{
		qWarning() << "Using nearest format";
		format = info.nearestFormat(format);
		qWarning() << format.sampleRate();
		qWarning() << format.channelCount();
		qWarning() << format.sampleSize();
		qWarning() << format.codec();
		qWarning() << format.byteOrder();
		qWarning() << format.sampleType();
	}

	m_output = new QAudioOutput(format, this);
	m_audioDevice = m_output->start();
}

MainWindow::~MainWindow()
{
}

void MainWindow::ConnectToTeamspeak()
{
	m_processor.Start(m_audioDevice);
}