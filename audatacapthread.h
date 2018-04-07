#ifndef AUDATACAPTHREAD_H
#define AUDATACAPTHREAD_H

#include <QObject>
#include <QThread>
#include <QAudioInput>
#include <QBuffer>
#include <QByteArray>
#include "audataringbuffer.h"
#include "timestamp.h"

class AuDataCapThread : public QObject
{
    Q_OBJECT
public:
    explicit AuDataCapThread(TimeStamp *_timestamp,             //全局的时间戳对象
                             AuDataRingBuffer *_rawRingBuffer,  //全局的音频数据环形缓冲区
                             QObject *parent = nullptr);

    bool setAudioFmt(const QAudioFormat &);
    QAudioFormat getAudioFmt();

    bool start();
    bool stop();
    QAudioInput *getAudioInput(){
        return qtaudioinput;
    }
signals:

public slots:
    void slot_pushData2Buf();
private:
    QThread *thread;            //本线程
    QAudioInput *qtaudioinput;
    QIODevice *qtaudiodev;
    QAudioFormat qtformat;

    AuDataRingBuffer *rawRingBuffer;  //全局的音频数据环形缓冲区
    TimeStamp *timestamp;  //全局的时间戳对象 ,删除

    qint64 lastTimestamp;
};

#endif // AUDATACAPTHREAD_H
