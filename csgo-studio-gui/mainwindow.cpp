#include "mainwindow.h"

#include <QAudioFormat>
#include <QAudioOutput>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QToolbar>
#include <QTreeWidget>

MainWindow::MainWindow()
	: m_output(nullptr)
{
	QToolBar* toolbar = new QToolBar(this);
	toolbar->addAction(tr("Connect"), [this]() { ConnectToTeamspeak(); });
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);

	QWidget* central = new QWidget(this);
	QHBoxLayout* centralLayout = new QHBoxLayout(central);
	central->setLayout(centralLayout);
	setCentralWidget(central);

	QTreeWidget* tree = new QTreeWidget(central);
	tree->setWindowTitle(tr("Channels"));
	tree->headerItem()->setText(0, "Studio Channels");
	tree->headerItem()->setText(1, "ID");
	tree->setDragEnabled(true);
	tree->setDragDropMode(QAbstractItemView::InternalMove);
	tree->setDropIndicatorShown(true);
	auto addChannel = [tree](const QString& text) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, text);
		item->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
		tree->addTopLevelItem(item);
	};
	addChannel(tr("General"));
	for(int i  = 1; i <= 5; ++i)
		addChannel(tr("Channel %1").arg(i));
	tree->expandAll();

	centralLayout->addWidget(tree);

	QVBoxLayout* generalLayout = new QVBoxLayout();

	// Properties panel
	QFrame* properties = new QFrame(central);
	QHBoxLayout* propertiesLayout = new QHBoxLayout(properties);
	properties->setFrameShape(QFrame::HLine);
	properties->setFrameShadow(QFrame::Sunken);
	properties->setWindowTitle(tr("Properties"));

	QSlider* delay = new QSlider();
	delay->setOrientation(Qt::Orientation::Horizontal);
	delay->setMinimum(0);
	delay->setMaximum(5 * 60); // 5 min

	QLabel* delayText = new QLabel(tr("Delay: 0s"));
	connect(delay, &QSlider::valueChanged, [this, delayText](int val) {
		delayText->setText(QString(tr("Delay: %1s")).arg(val));
		m_processor.SetDelay(val);
		});

	propertiesLayout->addWidget(delay);
	propertiesLayout->addWidget(delayText);
	properties->setLayout(propertiesLayout);

	QFrame* channels = new QFrame(central);
	channels->setFrameShape(QFrame::HLine);
	channels->setFrameShadow(QFrame::Sunken);
	channels->setWindowTitle(tr("Channels"));
	QGridLayout* channelsLayout = new QGridLayout();
	auto addButton = [channelsLayout](const QString& text, int row, int col) {
		QPushButton* button = new QPushButton();
		button->setText(text);
		button->setCheckable(true);
		channelsLayout->addWidget(button,row, col);
	};

	addButton(tr("General"), 0, 0);
	for (int i = 1; i <= 5; ++i)
		addButton(tr("Channel %1").arg(i), i / 2, i % 2);
	channels->setLayout(channelsLayout);

	generalLayout->addWidget(properties);
	generalLayout->addWidget(channels);

	QSpacerItem* spacer = new QSpacerItem(0,0, QSizePolicy::Minimum,QSizePolicy::Expanding);
	generalLayout->addSpacerItem(spacer);

	centralLayout->addLayout(generalLayout);


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

	connect(&m_processor, &AudioProcessor::OnClientInfoReceived, this, &MainWindow::OnClientConnected);
	connect(&m_processor, &AudioProcessor::OnClientInfoReceived, [this, tree](quint16 clientId, QString name) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, name);
		item->setText(1, QString::number(clientId));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
		tree->topLevelItem(0)->addChild(item);
		tree->expandAll();
		emit OnClientJoinedChannel(clientId, 0);
	});
	connect(&m_processor, &AudioProcessor::OnInfo, this, [](QString str) { qInfo() << str; });
	connect(&m_processor, &AudioProcessor::OnWarning, this, [](QString str) { qWarning() << str; });
	connect(&m_processor, &AudioProcessor::OnError, this, [](QString str) { qCritical() << str; });
}

MainWindow::~MainWindow()
{
}

void MainWindow::ConnectToTeamspeak()
{
	m_processor.Start(m_audioDevice);
}

void MainWindow::OnClientConnected(quint16 clientId, QString name)
{
	qWarning() << name << " joined (id: " << clientId << ")";
}