#include "audataringbuffer.h"
#include <QDebug>

AuDataRingBuffer::AuDataRingBuffer()
{
    freeCount = new QSemaphore(BUF_COUNT);
    usedCount = new QSemaphore(0);
    ringBuffer.clear();
}

bool AuDataRingBuffer::pushData(const QByteArray &data,const qreal &_timestamp)
{
    if(freeCount->tryAcquire(1) == false){
        qDebug("#### audio raw buffer is full!");
        return false;
    }

    mutex.lock();
    AuRawData adata;
    adata.rawData = data;
    adata.timestamp = _timestamp;
    ringBuffer.append(adata);
    mutex.unlock();

    usedCount->release(1);
    return true;
}

// 返回 false 表示数据传输结束
bool AuDataRingBuffer::getData(QByteArray &data,qreal &_timestamp)
{
    usedCount->acquire(1);

    mutex.lock();
    AuRawData adata = ringBuffer.takeFirst();
    data = adata.rawData;
    _timestamp = adata.timestamp;
    mutex.unlock();

    freeCount->release(1);
//    qDebug()<<"free count"<<freeCount->available()
//           <<"used count"<<usedCount->available();
    return true;
}


