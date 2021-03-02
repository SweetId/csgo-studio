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
	if (m_processThread.joinable())
		m_processThread.join();
}

void AudioProcessor::SetDelay(int32_t delay)
{
	m_delay = delay * 1000; // seconds to msec
}

bool AudioProcessor::Start(QIODevice* audioDevice)
{
	bool bConnected = m_client.StartClient("127.0.0.1", 37015);
	if (bConnected)
	{
		m_client.OnSamplesReceived += [this](uint32_t clientId, uint32_t size, uint8_t* samples) {
			std::unique_lock<std::mutex> lk(m_samplesMutex);
			m_timedSamples.push_back({ QDateTime::currentMSecsSinceEpoch(), size, std::move(std::unique_ptr<uint8_t[]>(samples)) });
		};

		m_processThread = std::thread([this, audioDevice]() { RunAudio(audioDevice); });
	}
	return bConnected;
}

void AudioProcessor::RunAudio(QIODevice* audioDevice)
{
	m_bRunning = true;
	while (m_bRunning)
	{
		if (m_timedSamples.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
		std::list<Samples>::iterator nextSample;
		{
			std::unique_lock<std::mutex> lk(m_samplesMutex);
			nextSample = m_timedSamples.begin();
		}

		qint64 nextTimestamp = (nextSample->timestamp + m_delay);
		if (nextTimestamp > currentTime)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(currentTime - nextTimestamp));
			continue;
		}

		audioDevice->write((char*)nextSample->data.get(), nextSample->size);
		{
			std::unique_lock<std::mutex> lk(m_samplesMutex);
			m_timedSamples.pop_front();
		}
	}
}