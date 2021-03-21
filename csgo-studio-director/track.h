#pragma once

#include <QObject>
#include <QMutex>

template <typename TData>
class Track
{
public:
	bool IsMuted() const { return false; }

	void Add(qint64 timestamp, const TData& samples)
	{
		m_mutex.lock();
		m_timedSamples.push_back({ timestamp, samples });
		m_mutex.unlock();
	}

	void GetBetween(qint64 from, qint64 to, QVector<TData>& out)
	{
		m_mutex.lock();
		for (auto& sample : m_timedSamples)
		{
			if (sample.timestamp >= from && sample.timestamp <= to)
			{
				out.push_back(sample.data);
			}
		}
		m_mutex.unlock();
	}

private:
	struct Samples
	{
		qint64 timestamp;
		TData data;
	};

	QMutex m_mutex;
	QVector<Samples> m_timedSamples;
};