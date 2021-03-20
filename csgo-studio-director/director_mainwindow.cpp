#include "director_mainwindow.h"

#include "track.h"

#include <QHBoxLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>

MainWindow::MainWindow()
	: m_serverTree(nullptr)
{
	QToolBar* toolbar = new QToolBar(this);
	toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
	toolbar->setAllowedAreas(Qt::ToolBarArea::AllToolBarAreas);
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);

	// Network connection
	toolbar->addWidget(new QLabel("Server IP:", toolbar));
	QLineEdit* serveripText = new QLineEdit(toolbar);
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
	for (int i = 1; i <= 5; ++i)
		addChannel(tr("Channel %1").arg(i));
	m_serverTree->expandAll();

	centralLayout->addWidget(m_serverTree);

	connect(&m_connection, &QNetClient::ConnectedToServer, this, &MainWindow::ConnectedToServer);
	connect(&m_connection, &QNetClient::DisconnectedFromServer, this, &MainWindow::DisconnectedFromServer);
	connect(&m_connection, &QNetClient::ErrorOccured, this, &MainWindow::OnSocketErrorOccurred);

	connect(&m_connection, &QNetClient::ClientIdentifierReceived, this, &MainWindow::OnClientIdentifierReceived);
	connect(&m_connection, &QNetClient::MicrophoneSamplesReceived, this, &MainWindow::OnMicrophoneSamplesReceived);
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

void MainWindow::OnClientIdentifierReceived(const QNetClientIdentifier& identifier)
{
	auto it = m_tracks.find(identifier.id);
	if (it == m_tracks.end())
	{
		m_tracks[identifier.id] = new QTrack(identifier.id, identifier.name, this);
		it = m_tracks.find(identifier.id);
	}
	else
	{
		it.value()->SetName(identifier.name);
	}

	auto items = m_serverTree->findItems(QString::number(it.key()), Qt::MatchExactly | Qt::MatchRecursive, 1);
	if (items.isEmpty())
	{
		// insert new client
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, it.value()->GetName());
		item->setText(1, QString::number(it.key()));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
		m_serverTree->topLevelItem(0)->addChild(item);
		m_serverTree->expandAll();
	}
	else
	{
		items[0]->setText(0, it.value()->GetName());
	}
}

void MainWindow::OnMicrophoneSamplesReceived(const QNetSoundwave& header, const QByteArray& samples)
{

}