#ifndef VIDEORINGBUFFER_H
#define VIDEORINGBUFFER_H
#include <QSemaphore>
#include <QMutex>
#include <QList>
#include <QByteArray>
#include <QImage>
#define VIDEO_BUF_COUNT 10

struct VideoRawData
{
    VideoRawData() {
        timestamp = 0;
    }
    QImage image; //原始数据
    qreal timestamp;   //时间戳  cur-base
};

class VideoRingBuffer
{
public:
    VideoRingBuffer();
    bool pushData(const QImage &data,const qreal &_timestamp); //push不了，会返回false
    bool getData(QImage &data,qreal &_timestamp); //get不了会休眠
    bool isEmptyed();
private:
    QList<VideoRawData> ringBuffer;
    QSemaphore freeCount;
    QSemaphore usedCount;
    QMutex mutex;
};

#endif // VIDEORINGBUFFER_H
