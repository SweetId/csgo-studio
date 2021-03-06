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
	, m_serverTree(nullptr)
{
	QToolBar* toolbar = new QToolBar(this);
	toolbar->addAction(tr("Connect"), [this]() { ConnectToTeamspeak(); });
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);

	QWidget* central = new QWidget(this);
	QHBoxLayout* centralLayout = new QHBoxLayout(central);
	central->setLayout(centralLayout);
	setCentralWidget(central);

	m_serverTree = new QTreeWidget(central);
	m_serverTree->setWindowTitle(tr("Channels"));
	m_serverTree->headerItem()->setText(0, "Studio Channels");
	m_serverTree->headerItem()->setText(1, "ID");
	m_serverTree->setDragEnabled(true);
	m_serverTree->setDragDropMode(QAbstractItemView::InternalMove);
	m_serverTree->setDropIndicatorShown(true);
	auto addChannel = [this](const QString& text) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, text);
		item->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
		m_serverTree->addTopLevelItem(item);
	};
	addChannel(tr("General"));
	for(int i  = 1; i <= 5; ++i)
		addChannel(tr("Channel %1").arg(i));
	m_serverTree->expandAll();

	centralLayout->addWidget(m_serverTree);

	QVBoxLayout* generalLayout = new QVBoxLayout();

	// Properties panel
	QFrame* properties = new QFrame(central);
	QGridLayout* propertiesLayout = new QGridLayout(properties);
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

	delay->setValue(10);

	QPushButton* refreshClients = new QPushButton(tr("Refresh Clients"), properties);
	connect(refreshClients, &QPushButton::clicked, &m_processor, &AudioProcessor::RequestClientsInfos);

	propertiesLayout->addWidget(delay, 0, 0);
	propertiesLayout->addWidget(delayText, 0, 1);
	propertiesLayout->addWidget(refreshClients, 1, 0);
	properties->setLayout(propertiesLayout);

	QFrame* channels = new QFrame(central);
	channels->setFrameShape(QFrame::HLine);
	channels->setFrameShadow(QFrame::Sunken);
	channels->setWindowTitle(tr("Channels"));
	QGridLayout* channelsLayout = new QGridLayout();
	auto addButton = [this, channelsLayout](const QString& text, int row, int col) {
		QPushButton* button = new QPushButton();
		button->setText(text);
		button->setCheckable(true);
		channelsLayout->addWidget(button,row, col);

		connect(button, &QPushButton::toggled, [this, text](bool bToggled) {
			emit ChannelToggled(text, bToggled);
		});
	};

	connect(this, &MainWindow::ChannelToggled, this, &MainWindow::OnChannelToggled);

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
	connect(&m_processor, &AudioProcessor::OnClientInfoReceived, [this](quint16 clientId, QString name) {
		auto items = m_serverTree->findItems(QString::number(clientId), Qt::MatchExactly | Qt::MatchRecursive, 1);
		if (items.isEmpty())
		{
			// insert new client
			QTreeWidgetItem* item = new QTreeWidgetItem();
			item->setText(0, name);
			item->setText(1, QString::number(clientId));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
			m_serverTree->topLevelItem(0)->addChild(item);
			m_serverTree->expandAll();
			emit ClientJoinedChannel(clientId, 0);
		}
		else
		{
			items[0]->setText(0, name);
		}
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
	RefreshEnabledChannels();
	qWarning() << name << " joined (id: " << clientId << ")";
}

void MainWindow::OnChannelToggled(QString name, bool bToggled)
{
	if (!bToggled)
		m_enabledChannels.remove(name);
	else
		m_enabledChannels.insert(name);

	RefreshEnabledChannels();
}

void MainWindow::RefreshEnabledChannels()
{
	std::set<uint16_t> clientsInEnabledChannels;
	for (const QString& channel : m_enabledChannels)
	{
		QList<QTreeWidgetItem*> items = m_serverTree->findItems(channel, Qt::MatchExactly, 0);
		if (!items.isEmpty())
		{
			for (int i = 0; i < items[0]->childCount(); ++i)
			{
				clientsInEnabledChannels.insert(items[0]->child(i)->text(1).toInt());
			}
		}
	}

	m_processor.SetEnabledClients(clientsInEnabledChannels);
}