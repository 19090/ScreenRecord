#include "videoringbuffer.h"
#include <QDebug>

VideoRingBuffer::VideoRingBuffer()
:freeCount(VIDEO_BUF_COUNT),usedCount(0)
{
    ringBuffer.clear();
}

bool VideoRingBuffer::pushData(const QImage &data,const qreal &_timestamp)
{
    if(freeCount.tryAcquire(1) == false){
        //qDebug("#### video raw buffer is full!");
        return false;
    }
    //qDebug("#### video raw buffer is not full!");
    mutex.lock();
    VideoRawData adata;
    adata.image = data;
    adata.timestamp = _timestamp;
    ringBuffer.append(adata);
    mutex.unlock();

    usedCount.release(1);

    return false;
}

bool VideoRingBuffer::getData(QImage &data,qreal &_timestamp)
{
    if(usedCount.tryAcquire(1) == false){
        return false;
    }
    mutex.lock();
    VideoRawData adata = ringBuffer.takeFirst();
    data = adata.image;
    _timestamp = adata.timestamp;
    mutex.unlock();
    freeCount.release(1);

    return true;
}

bool VideoRingBuffer::isEmptyed()
{
    return ringBuffer.isEmpty();
}
