#pragma once

#include <QObject>

#include "multi_track.h"

#include <QByteArray>
#include <QSet>
#include <QThread>

class QMultiTrackPlayerWorker : public QThread
{
	Q_OBJECT

public:
	QMultiTrackPlayerWorker(class QMultiTracksPlayer* player);

	void run() override;
	qint64 GetCurrentTimecode() const { return m_currentTimecode; }

public slots:
	void Play();
	void Pause();
	void Restart();

	void JumpToTimecode(qint64 timecode);

	void SetAudioDevice(class QIODevice* audioDevice);

private:
	class QMultiTracksPlayer* m_player;
	class QIODevice* m_audioDevice;

	qint64 m_currentTimecode;
	qint64 m_playTimestamp;
	bool m_bPaused;
};

class QMultiTracksPlayer : public QObject
{
	Q_OBJECT

public:
	QMultiTracksPlayer(QObject* parent = nullptr);
	~QMultiTracksPlayer();

	void AddMultiTrack(class QMultiTrack* multitracks) { m_tracks.insert(multitracks); }
	void RemoveMultitrack(class QMultiTrack* multitracks) { m_tracks.remove(multitracks); }

	void Play();
	void Pause();
	void Restart();

	void SetAudioDevice(class QIODevice* audioDevice);

	void JumpToTimecode(qint64 timecode);
	qint64 GetCurrentTimecode() const;

signals:
	void OnPlay();
	void OnPause();
	void OnRestart();
	void OnAudioDeviceChanged(class QIODevice*);
	void OnJumpToTimecode(qint64 timecode);

private:
	QMultiTrackPlayerWorker* m_playerThread;
	QSet<class QMultiTrack*> m_tracks;

	friend class QMultiTrackPlayerWorker;
};