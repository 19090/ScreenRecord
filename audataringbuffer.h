#ifndef AUDATARINGBUFFER_H
#define AUDATARINGBUFFER_H

#include <QSemaphore>
#include <QMutex>
#include <QList>
#include <QByteArray>

#define BUF_COUNT 60

/*
 * audio raw 数据的环形缓冲区
*/

struct AuRawData
{
    AuRawData() {
        rawData.clear();
        timestamp = 0;
    }
    QByteArray rawData; //原始数据
    qreal timestamp;   //时间戳  cur-base
};

class AuDataRingBuffer
{
public:
    AuDataRingBuffer();
    bool pushData(const QByteArray &data,const qreal &_timestamp); //push不了，会返回false
    bool getData(QByteArray &data,qreal &_timestamp); //get不了会休眠
            /* getData 返回false 表示收到end信息 */
    void clear(){
        delete freeCount;
        delete usedCount;
        freeCount = new QSemaphore(BUF_COUNT);
        usedCount = new QSemaphore(0);
        this->ringBuffer.clear();
    }
private:
    QList<AuRawData> ringBuffer;
    QSemaphore *freeCount;
    QSemaphore *usedCount;
    QMutex mutex;
};

#endif // AUDATARINGBUFFER_H
