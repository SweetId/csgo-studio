#pragma once

#include <QDateTime>
#include <QObject>

class QClock : public QObject
{
	Q_OBJECT

public:
	QClock(QObject* parent = nullptr)
		: QObject(parent)
		, m_startTime(0)
		, m_pauseTime(0)
	{}

	void Start()
	{
		m_startTime = QDateTime::currentMSecsSinceEpoch();
		m_pauseTime = 0;
	}

	void Pause()
	{
		m_pauseTime = QDateTime::currentMSecsSinceEpoch();
	}

	void SetElapsed(qint64 elapsed)
	{
		m_startTime = QDateTime::currentMSecsSinceEpoch() - elapsed;
		if(IsPaused())
			m_pauseTime = m_startTime + elapsed;
	}
	
	void Resume()
	{
		if (IsPaused())
		{
			m_startTime += (QDateTime::currentMSecsSinceEpoch() - m_pauseTime);
			m_pauseTime = 0;
		}
	}

	qint64 Get() const
	{
		if (IsPaused())
		{
			return m_pauseTime - m_startTime;
		}
		else
		{
			return QDateTime::currentMSecsSinceEpoch() - m_startTime;
		}
	}

	inline bool IsPaused() const { return m_pauseTime > 0; }

private:
	qint64 m_startTime;
	qint64 m_pauseTime;
};