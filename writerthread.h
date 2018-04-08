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

    explicit WriterThread(PacketRingBuffer *_aupacketBuffer,
                          PacketRingBuffer *_avpacketBuffer,
                          QtAV::AudioEncoder *_auEnc,
                          QtAV::VideoEncoder *_viEnc,
                          QThread *parent = nullptr);
    explicit WriterThread(PacketRingBuffer *_packetBuffer,
                          QtAV::AudioEncoder *_auEnc,
                          QtAV::VideoEncoder *_viEnc,
                          QThread *parent = nullptr);




    bool start();
    bool stop();
    void setRecordVideo(bool);
    void setRecordAudio(bool);
    void setSaveFileName(const QString &_filename);
signals:

public slots:

protected:
    void run();
private:
    QtAV::AVMuxer *muxer;
    QString saveFileName;
#ifdef USE_TWO_PACKET_BUF
    PacketRingBuffer *aupacketBuffer;
    PacketRingBuffer *avpacketBuffer;
#else
        PacketRingBuffer *packetBuffer;
#endif

#ifdef SPLIT_TO_FILE
    QString audiofile,videofile;

    QtAV::AVMuxer *audioMuxer;
    QtAV::AVMuxer *videoMuxer;

#endif

    bool isRecordAudio;
    bool isRecordVideo;
};

#endif // WRITERTHREAD_H
