#include "audio_processor.h"

#include <QDateTime>
#include <QIODevice>

#include "tcp_data.h"

AudioProcessor::AudioProcessor()
	: m_bRunning({ false })
	, m_delay(0)
{ }

AudioProcessor::~AudioProcessor()
{
	m_bRunning = false;
	if (m_networkThread.joinable())
		m_networkThread.join();
	if (m_processThread.joinable())
		m_processThread.join();
}

void AudioProcessor::SetDelay(int32_t delay)
{
	m_delay = delay * 1000; // seconds to msec
}

bool AudioProcessor::Start(QIODevice* audioDevice)
{
	bool bConnected = m_client.Connect("127.0.0.1", 37015);
	if (bConnected)
	{
		m_bRunning = true;
		m_networkThread = std::thread([this]() { RunClient(); });
		m_processThread = std::thread([this, audioDevice]() { RunAudio(audioDevice); });
	}
	return bConnected;
}

void AudioProcessor::RunClient()
{
	while (m_bRunning)
	{
		OutgoingSoundDataHeader header;
		int64_t ret = m_client.Recv(&header, sizeof(OutgoingSoundDataHeader));

		if (ret > 0)
		{
			std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(header.samplesSize);
			ret = m_client.Recv(data.get(), header.samplesSize);
			if (ret > 0)
			{
				m_timedSamples.push_back({ QDateTime::currentMSecsSinceEpoch(), header.samplesSize, std::move(data) });
			}
		}

	}
}

void AudioProcessor::RunAudio(QIODevice* audioDevice)
{
	while (m_bRunning)
	{
		if (m_timedSamples.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
		auto nextSample = m_timedSamples.begin();

		qint64 nextTimestamp = (nextSample->timestamp + m_delay);
		if (nextTimestamp > currentTime)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(currentTime - nextTimestamp));
			continue;
		}

		audioDevice->write((char*)nextSample->data.get(), nextSample->size);
		m_timedSamples.pop_front();
	}
}