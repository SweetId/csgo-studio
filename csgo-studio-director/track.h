#include <QObject>

class QTrack : public QObject
{
	Q_OBJECT

public:
	QTrack(quint32 id, const QString& name, QObject* parent = nullptr);

	quint32 GetId() const { return m_id; }
	
	const QString& GetName() const { return m_name; }
	void SetName(const QString& name) { m_name = name; }

private:
	quint32 m_id;
	QString m_name;
};