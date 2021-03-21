#pragma once

#include <qglobal.h>

#include <QAudioFormat>
#include <QDataStream>

struct QNetClientIdentifier
{
	static const quint32 Type = 1;
	static const bool HasData = false;
	quint32 id;
	QString name;
};
struct QNetSoundwave
{
	static const quint32 Type = 2;
	static const bool HasData = true;
	quint32 id;
	quint32 size;
	qint64 timestamp;
};
struct QNetCameraFrame
{
	static const quint32 Type = 3;
	static const bool HasData = true;
	quint32 id;
	quint32 size;
	qint64 timestamp;
};
struct QNetServerSession
{
	static const quint32 Type = 4;
	static const bool HasData = false;
	qint64 timestamp;
};

inline QDataStream& operator>>(QDataStream& stream, QNetClientIdentifier& header)
{
	stream >> header.id >> header.name;
	return stream;
}
inline QDataStream& operator<<(QDataStream& stream, const QNetClientIdentifier& header)
{
	stream << header.id << header.name;
	return stream;
}

inline QDataStream& operator>>(QDataStream& stream, QNetSoundwave& header)
{
	stream >> header.id >> header.size >> header.timestamp;
	return stream;
}
inline QDataStream& operator<<(QDataStream& stream, const QNetSoundwave& header)
{
	stream << header.id << header.size << header.timestamp;
	return stream;
}

inline QDataStream& operator>>(QDataStream& stream, QNetCameraFrame& header)
{
	stream >> header.id >> header.size >> header.timestamp;
	return stream;
}
inline QDataStream& operator<<(QDataStream& stream, const QNetCameraFrame& header)
{
	stream << header.id << header.size << header.timestamp;
	return stream;
}

inline QDataStream& operator>>(QDataStream& stream, QNetServerSession& header)
{
	stream >> header.timestamp;
	return stream;
}
inline QDataStream& operator<<(QDataStream& stream, const QNetServerSession& header)
{
	stream << header.timestamp;
	return stream;
}