#include "multi_tracks_player.h"

#include <QDateTime>
#include <QIODevice>

QMultiTrackPlayerWorker::QMultiTrackPlayerWorker(class QMultiTracksPlayer* player)
	: QThread(player)
	, m_player(player)
	, m_audioDevice(nullptr)
{}

void QMultiTrackPlayerWorker::run()
{
	m_clock.Start();
	m_clock.Pause();
	qint64 lastTimecode = m_clock.Get();

	while (!isInterruptionRequested())
	{
		if (m_clock.IsPaused())
		{
			QThread::msleep(100);
			continue;
		}

		qint64 currentTimecode = m_clock.Get();

		for (auto* multitrack : m_player->m_tracks)
		{
			if (nullptr != m_audioDevice)
			{
				auto* audio = multitrack->GetAudioTrack();
				if (nullptr != audio && !audio->IsMuted())
				{
					QVector<QByteArray> samples;
					audio->GetBetween(lastTimecode, currentTimecode, samples);

					QByteArray data;
					for (QByteArray& sample : samples)
						data.append(sample);
					samples.empty();

					m_audioDevice->write(data);
				}
			}
		}
		lastTimecode = currentTimecode;
	}
}

void QMultiTrackPlayerWorker::Play()
{
	m_clock.Start();
}

void QMultiTrackPlayerWorker::Pause()
{
	m_clock.Pause();
}

void QMultiTrackPlayerWorker::Resume()
{
	m_clock.Resume();
}

void QMultiTrackPlayerWorker::JumpToTimecode(qint64 timecode)
{
	m_clock.SetElapsed(timecode);
}

void QMultiTrackPlayerWorker::SetAudioDevice(QIODevice* audioDevice) { m_audioDevice = audioDevice; }

QMultiTracksPlayer::QMultiTracksPlayer(QObject* parent)
	: QObject(parent)
	, m_playerThread(new QMultiTrackPlayerWorker(this))
{
	connect(this, &QMultiTracksPlayer::OnPlay, m_playerThread, &QMultiTrackPlayerWorker::Play);
	connect(this, &QMultiTracksPlayer::OnPause, m_playerThread, &QMultiTrackPlayerWorker::Pause);
	connect(this, &QMultiTracksPlayer::OnResume, m_playerThread, &QMultiTrackPlayerWorker::Resume);
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

void QMultiTracksPlayer::Resume()
{
	emit OnResume();
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