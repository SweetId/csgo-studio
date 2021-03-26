#include "director_mainwindow.h"

#include "multi_track.h"
#include "multi_tracks_player.h"
#include "ffmpeg/stream_decoder.h"

#include <QAudioFormat>
#include <QAudioOutput>
#include <QComboBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace
{
	template <typename T>
	T Lerp(T a, T b, double t) { return a + t * (b - a); }
}

MainWindow::MainWindow()
	: m_player(new QMultiTracksPlayer(this))
	, m_serverTree(nullptr)
	, m_initialTimestamp(0)
{
	QAudioFormat format;
	// Set up the format, eg.
	format.setSampleRate(44100);
	format.setChannelCount(2);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	QToolBar* toolbar = new QToolBar(this);
	toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
	toolbar->setAllowedAreas(Qt::ToolBarArea::AllToolBarAreas);
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);

	// Network connection
	toolbar->addWidget(new QLabel("Server IP:", toolbar));
	QLineEdit* serveripText = new QLineEdit("127.0.0.1", toolbar);
	toolbar->addWidget(serveripText);

	QAction* networkOnAction = new QAction(QIcon(":/icons/network_off.png"), tr("Connect to server"), this);
	QAction* networkOffAction = new QAction(QIcon(":/icons/network_on.png"), tr("Disconnect from server"), this);
	QToolButton* networkButton = new QToolButton(toolbar);
	networkButton->setDefaultAction(networkOnAction);
	toolbar->addWidget(networkButton);
	connect(this, &MainWindow::ConnectingToServer, [networkButton, serveripText]() {
		networkButton->setEnabled(false);
		serveripText->setEnabled(false);
		});

	connect(networkOnAction, &QAction::triggered, [this, serveripText](bool bTriggered) { ConnectToServer(serveripText->text()); });
	connect(networkOffAction, &QAction::triggered, this, &MainWindow::DisconnectFromServer);

	connect(this, &MainWindow::ConnectedToServer, [networkButton, networkOffAction]() {
		networkButton->setDefaultAction(networkOffAction);
		networkButton->setEnabled(true);
		});
	connect(this, &MainWindow::DisconnectedFromServer, [networkButton, networkOnAction, serveripText]() {
		networkButton->setDefaultAction(networkOnAction);
		serveripText->setEnabled(true);
		});

	// Microphone list and button
	toolbar->addSeparator();
	QComboBox* speakersList = new QComboBox(toolbar);
	{
		QSet<QString> items;
		for (const QAudioDeviceInfo& speaker : QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput))
			items.insert(speaker.deviceName());
		speakersList->addItems(items.values());
	}
	toolbar->addWidget(speakersList);
	connect(speakersList, &QComboBox::currentTextChanged, [this, format](const QString& name) {
		for (const QAudioDeviceInfo& speaker : QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput))
		{
			if (speaker.deviceName() == name && speaker.isFormatSupported(format))
			{
				m_audioOutput = new QAudioOutput(speaker, format, this);
				m_audioDevice = m_audioOutput->start();
				m_player->SetAudioDevice(m_audioDevice);
				break;
			}
		}
	});

	QWidget* central = new QWidget(this);
	QVBoxLayout* centralLayout = new QVBoxLayout(central);
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
	for (int i = 1; i <= 5; ++i)
		addChannel(tr("Channel %1").arg(i));
	m_serverTree->expandAll();

	centralLayout->addWidget(m_serverTree);

	QHBoxLayout* playerControlBar = new QHBoxLayout(this);
	QToolButton* playButton = new QToolButton(this);
	QAction* playAction = new QAction(QIcon(":/icons/play.png"), tr("Play"), this);
	QAction* pauseAction = new QAction(QIcon(":/icons/pause.png"), tr("Pause"), this);
	playButton->setDefaultAction(playAction);
	playerControlBar->addWidget(playButton);

	connect(playAction, &QAction::triggered, [playButton, pauseAction]() { playButton->setDefaultAction(pauseAction); });
	connect(pauseAction, &QAction::triggered, [playButton, playAction]() { playButton->setDefaultAction(playAction); });
	connect(playAction, &QAction::triggered, [this]() { m_player->Resume(); });
	connect(pauseAction, &QAction::triggered, [this]() { m_player->Pause(); });


	QSlider* timeSlider = new QSlider(Qt::Orientation::Horizontal, this);
	timeSlider->setRange(0, 100);
	playerControlBar->addWidget(timeSlider);

	QVBoxLayout* timingsLayout = new QVBoxLayout(this);
	QLabel* realTimeLabel = new QLabel("00:00:00/00:00:00", this);
	QLabel* relativeTimeLabel = new QLabel("00:00:00/00:00:00", this);
	timingsLayout->addWidget(realTimeLabel);
	timingsLayout->addWidget(relativeTimeLabel);
	playerControlBar->addLayout(timingsLayout);

	auto UpdateTimeLabels = [this, realTimeLabel, relativeTimeLabel]() {
		quint64 initialTime = m_initialTimestamp;
		quint64 currentTime = QDateTime::currentMSecsSinceEpoch();
		quint64 playerTime = m_player->GetCurrentTimecode();
		realTimeLabel->setText(QDateTime::fromMSecsSinceEpoch(initialTime + playerTime).toString("HH:mm:ss") + "/" + QDateTime::fromMSecsSinceEpoch(currentTime).toString("HH:mm:ss"));
		relativeTimeLabel->setText(QDateTime::fromMSecsSinceEpoch(playerTime).toString("HH:mm:ss") + "/" + QDateTime::fromMSecsSinceEpoch(currentTime - initialTime).toString("HH:mm:ss"));
	};


	connect(timeSlider, &QSlider::valueChanged, [this, UpdateTimeLabels](int val) {
		auto lval = ::Lerp(m_initialTimestamp, QDateTime::currentMSecsSinceEpoch(), val / 100.f);
		m_player->JumpToTimecode(lval - m_initialTimestamp);
		UpdateTimeLabels();
	});
	connect(this, &MainWindow::InitialTimestampChanged, [this, UpdateTimeLabels](quint64) {
		UpdateTimeLabels();
	});
	QTimer* timeUpdater = new QTimer(this);
	connect(timeUpdater, &QTimer::timeout, UpdateTimeLabels);
	timeUpdater->start(1000);

	centralLayout->addLayout(playerControlBar);

	connect(&m_connection, &QNetClient::ConnectedToServer, this, &MainWindow::ConnectedToServer);
	connect(&m_connection, &QNetClient::DisconnectedFromServer, this, &MainWindow::DisconnectedFromServer);
	connect(&m_connection, &QNetClient::ErrorOccured, this, &MainWindow::OnSocketErrorOccurred);

	connect(&m_connection, &QNetClient::ClientIdentifierReceived, this, &MainWindow::OnClientIdentifierReceived);
	connect(&m_connection, &QNetClient::MicrophoneSamplesReceived, this, &MainWindow::OnMicrophoneSamplesReceived);
	connect(&m_connection, &QNetClient::ServerSessionReceived, this, &MainWindow::OnServerSessionReceived);

	QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
	m_audioOutput = new QAudioOutput(info, format, this);
	m_audioDevice = m_audioOutput->start();

	m_player->SetAudioDevice(m_audioDevice);

	AudioSampleDescriptor descriptor(ECodec::Mp2, 2, 64000, 44100, ESampleFormat::S16);
	m_decoder = new StreamDecoder(descriptor);
}

MainWindow::~MainWindow()
{
}

void MainWindow::ConnectToServer(const QString& serverip)
{
	if (serverip.trimmed().isEmpty())
	{
		QMessageBox::warning(this, "Error", "Please enter valid ip");
		return;
	}

	QStringList server = serverip.split(':');
	QString ip;
	qint16 port = (quint16)37026;
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

void MainWindow::OnClientIdentifierReceived(const QNetClientIdentifier& header)
{
	QMultiTrack* track = GetOrCreateMultiTrack(header.id);
	track->SetName(header.name);

	auto items = m_serverTree->findItems(QString::number(track->GetId()), Qt::MatchExactly | Qt::MatchRecursive, 1);
	if (items.isEmpty())
	{
		// insert new client
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, track->GetName());
		item->setText(1, QString::number(track->GetId()));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
		m_serverTree->topLevelItem(0)->addChild(item);
		m_serverTree->expandAll();
	}
	else
	{
		items[0]->setText(0, track->GetName());
	}
}

void MainWindow::OnMicrophoneSamplesReceived(const QNetSoundwave& header, const QByteArray& samples)
{
	QByteArray outData;
	m_decoder->Decode(samples, outData);

	QMultiTrack* track = GetOrCreateMultiTrack(header.id);
	track->AddAudioSample(header.timestamp, outData);
}

void MainWindow::OnServerSessionReceived(const QNetServerSession& header)
{
	m_initialTimestamp = header.timestamp;
	emit InitialTimestampChanged(m_initialTimestamp);
}

QMultiTrack* MainWindow::GetOrCreateMultiTrack(quint32 id)
{
	auto it = m_tracks.find(id);
	if (it != m_tracks.end())
		return it.value();

	m_tracks[id] = new QMultiTrack(id, "", this);
	m_player->AddMultiTrack(m_tracks[id]);
	return m_tracks[id];
}