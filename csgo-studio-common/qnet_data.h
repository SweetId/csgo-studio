#pragma once

#include <qglobal.h>

#include <QAudioFormat>


struct QNetSoundwave
{
	static const quint32 Type = 2;
	quint32 id;
	quint32 size;
};
struct QNetCameraFrame
{
	static const quint32 Type = 3;
	quint32 id;
	quint32 size;
};

inline QDataStream& operator>>(QDataStream& stream, QNetSoundwave& header)
{
	stream >> header.id >> header.size;
	return stream;
}
inline QDataStream& operator<<(QDataStream& stream, const QNetSoundwave& header)
{
	stream << header.id << header.size;
	return stream;
}

inline QDataStream& operator>>(QDataStream& stream, QNetCameraFrame& header)
{
	stream >> header.id >> header.size;
	return stream;
}
inline QDataStream& operator<<(QDataStream& stream, const QNetCameraFrame& header)
{
	stream << header.id << header.size;
	return stream;
}