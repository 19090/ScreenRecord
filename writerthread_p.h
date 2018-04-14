#ifndef WRITERTHREAD_H
#define WRITERTHREAD_H

#include <QObject>
#include <QThread>

#include <QtAV/AVMuxer.h>
#include <QtAV/AudioEncoder.h>
#include <QtAV/VideoEncoder.h>
#include "packetringbuffer.h"

#undef  SPLIT_TO_FILE
#define USE_TWO_PACKET_BUF

class WriterThread : public QThread
{
    Q_OBJECT
public:
    explicit WriterThread(QThread *parent = nullptr);

    void setEncoder(QtAV::AudioEncoder *_auEnc,
                    QtAV::VideoEncoder *_viEnc);

    void setPacketBuffer(PacketRingBuffer *_aupacketBuffer,
                         PacketRingBuffer *_avpacketBuffer);


    bool start();
    bool stop();
    
    void setSaveFileName(const QString &_filename);
signals:

public slots:

protected:
    void run();
private:
    QtAV::AVMuxer *muxer;
    QString saveFileName;
    PacketRingBuffer *audioPacketBuffer;
    PacketRingBuffer *videoPacketBuffer;

    QtAV::AudioEncoder *audioEncoder;
    QtAV::VideoEncoder *videoEncoder;
};

#endif // WRITERTHREAD_H
