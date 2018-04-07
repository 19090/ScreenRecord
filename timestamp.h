#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <QObject>
#include <QMutex>

class TimeStamp
{
public:
    TimeStamp();
    void setTimeStamp(const qreal &_timestamp);
    qreal getTimeStamp();
private:
    qreal timestamp;
    QMutex mutex;
};

#endif // TIMESTAMP_H
