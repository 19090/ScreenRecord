#include "timestamp.h"

TimeStamp::TimeStamp()
{
    this->timestamp = 0;
}

void TimeStamp::setTimeStamp(const qreal &_timestamp)
{
    mutex.lock();
    this->timestamp = _timestamp;
    mutex.unlock();
}

qreal TimeStamp::getTimeStamp()
{
    qreal ret = 0;
    mutex.lock();
    ret = this->timestamp;
    mutex.unlock();
    return ret;
}
