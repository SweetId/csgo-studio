#pragma once

#include <QObject>
#include <QMutex>

template <typename TData>
class Track
{
public:
	struct Samples
	{
		qint64 timestamp;
		TData data;
	};

	bool IsMuted() const { return false; }

	void Add(qint64 timestamp, const TData& samples)
	{
		m_mutex.lock();
		// ordered insertion
		m_timedSamples.insert(
			std::upper_bound(m_timedSamples.begin(), m_timedSamples.end(), Samples{ timestamp, samples }, [](const Samples& a, const Samples& b) { return a.timestamp < b.timestamp; }),
			Samples{ timestamp, samples }
		);
		m_mutex.unlock();
	}

	void GetBetween(qint64 from, qint64 to, QVector<TData>& out)
	{
		m_mutex.lock();
		for (auto& sample : m_timedSamples)
		{
			if (sample.timestamp >= from && sample.timestamp < to)
			{
				out.push_back(sample.data);
			}
		}
		m_mutex.unlock();
	}

	typename QVector<Samples>::const_iterator begin() const
	{
		return m_timedSamples.begin();
	}

	typename QVector<Samples>::const_iterator end() const
	{
		return m_timedSamples.end();
	}

private:

	QMutex m_mutex;
	QVector<Samples> m_timedSamples;
};