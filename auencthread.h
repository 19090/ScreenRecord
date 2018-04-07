#ifndef AUENCTHREAD_H
#define AUENCTHREAD_H

#include <QObject>
#include <QThread>
#include <QtAV/AudioEncoder.h>
#include <QtAV/AudioFormat.h>
#include <QtAV/AudioFrame.h>
#include "audataringbuffer.h"
#include "packetringbuffer.h"

class AuEncThread : public QThread
{
    Q_OBJECT
public:
    explicit AuEncThread(AuDataRingBuffer *_rawDataBuffer,
                         PacketRingBuffer *_packetBuffer,
                         QThread *parent = nullptr);

    QtAV::AudioEncoder * getAudioEnc();

signals:

protected:
    void run();

public slots:
    void start();
    bool stop();
private:
    QtAV::AudioEncoder *auEnc;
    QtAV::AudioFormat *avAudioFormat;

    AuDataRingBuffer *rawDataBuffer;
    PacketRingBuffer *packetBuffer;

    //bool isExited;
};

#endif // AUENCTHREAD_H
