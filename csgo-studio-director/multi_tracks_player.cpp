#include "multi_tracks_player.h"

#include <QDateTime>
#include <QIODevice>

QMultiTrackPlayerWorker::QMultiTrackPlayerWorker(class QMultiTracksPlayer* player)
	: QThread(player)
	, m_player(player)
	, m_audioDevice(nullptr)
	, m_currentTimecode(0)
	, m_playTimestamp(QDateTime::currentMSecsSinceEpoch())
	, m_bPaused(true)
{}

void QMultiTrackPlayerWorker::run()
{
	while (!isInterruptionRequested())
	{
		while (m_bPaused)
		{
			QThread::msleep(100);
			continue;
		}

		qint64 lastTimecode = m_currentTimecode;
		m_currentTimecode = QDateTime::currentMSecsSinceEpoch() - m_playTimestamp;

		for (auto* multitrack : m_player->m_tracks)
		{
			if (nullptr != m_audioDevice)
			{
				auto* audio = multitrack->GetAudioTrack();
				if (nullptr != audio && !audio->IsMuted())
				{
					QVector<QByteArray> samples;
					audio->GetBetween(lastTimecode, m_currentTimecode, samples);

					QByteArray data;
					for (QByteArray& sample : samples)
						data.append(sample);
					samples.empty();

					m_audioDevice->write(data);
				}
			}
		}
	}
}

void QMultiTrackPlayerWorker::Play()
{
	m_bPaused = false;
	m_playTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void QMultiTrackPlayerWorker::Pause() { m_bPaused = true; }

void QMultiTrackPlayerWorker::Restart()
{
	Play();
	JumpToTimecode(0);
}

void QMultiTrackPlayerWorker::JumpToTimecode(qint64 timecode)
{
	m_currentTimecode = QDateTime::currentMSecsSinceEpoch() - timecode;
}

void QMultiTrackPlayerWorker::SetAudioDevice(QIODevice* audioDevice) { m_audioDevice = audioDevice; }

QMultiTracksPlayer::QMultiTracksPlayer(QObject* parent)
	: QObject(parent)
	, m_playerThread(new QMultiTrackPlayerWorker(this))
{
	connect(this, &QMultiTracksPlayer::OnPlay, m_playerThread, &QMultiTrackPlayerWorker::Play);
	connect(this, &QMultiTracksPlayer::OnPause, m_playerThread, &QMultiTrackPlayerWorker::Pause);
	connect(this, &QMultiTracksPlayer::OnRestart, m_playerThread, &QMultiTrackPlayerWorker::Restart);
	connect(this, &QMultiTracksPlayer::OnAudioDeviceChanged, m_playerThread, &QMultiTrackPlayerWorker::SetAudioDevice);
	connect(this, &QMultiTracksPlayer::OnJumpToTimecode, m_playerThread, &QMultiTrackPlayerWorker::JumpToTimecode);
	
	connect(m_playerThread, &QThread::finished, m_playerThread, &QObject::deleteLater);

	m_playerThread->start();
}

QMultiTracksPlayer::~QMultiTracksPlayer()
{
	m_playerThread->requestInterruption();
	m_playerThread->wait();
}

void QMultiTracksPlayer::Play()
{
	emit OnPlay();
}

void QMultiTracksPlayer::Pause()
{
	emit OnPause();
}

void QMultiTracksPlayer::Restart()
{
	emit OnRestart();
}

void QMultiTracksPlayer::SetAudioDevice(class QIODevice* audioDevice)
{
	emit OnAudioDeviceChanged(audioDevice);
}

void QMultiTracksPlayer::JumpToTimecode(qint64 timecode)
{
	emit OnJumpToTimecode(timecode);
}

qint64 QMultiTracksPlayer::GetCurrentTimecode() const
{
	return (m_playerThread != nullptr) ? m_playerThread->GetCurrentTimecode() : 0;
}