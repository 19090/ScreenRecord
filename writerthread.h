#ifndef WRITERTHREAD_H
#define WRITERTHREAD_H

#include <QObject>
#include <QThread>

#include <QtAV/AVMuxer.h>
#include <QtAV/AudioEncoder.h>
#include <QtAV/VideoEncoder.h>
#include "packetringbuffer.h"

#undef  SPLIT_TO_FILE

class WriterThread : public QThread
{
    Q_OBJECT
public:
    explicit WriterThread(PacketRingBuffer *_aupacketBuffer,PacketRingBuffer *_avpacketBuffer,QtAV::AudioEncoder *_auEnc,QtAV::VideoEncoder *_viEnc,QThread *parent = nullptr);

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

    PacketRingBuffer *aupacketBuffer;
    PacketRingBuffer *avpacketBuffer;

#ifdef SPLIT_TO_FILE
    QString audiofile,videofile;

    QtAV::AVMuxer *audioMuxer;
    QtAV::AVMuxer *videoMuxer;

#endif
};

#endif // WRITERTHREAD_H
