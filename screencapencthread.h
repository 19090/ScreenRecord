#ifndef SCREENCAPENCTHREAD_H
#define SCREENCAPENCTHREAD_H

#include <QObject>
#include <QThread>
#include <QScreen>
#include <QTimer>
#include <QDesktopWidget>

#include <QtAV/VideoEncoder.h>
#include <QtAV/VideoFormat.h>
#include <QtAV/VideoFrame.h>

#include "packetringbuffer.h"
#include "timestamp.h"

class ScreenCapEncThread : public QObject
{
    Q_OBJECT
public:
    explicit ScreenCapEncThread(TimeStamp *_timstamp,PacketRingBuffer *_ringBuffer,QObject *parent = nullptr);

    bool start();
    bool stop();
    QtAV::VideoEncoder * getVideoEnc();
signals:

public slots:
    void slot_timerTimeout();
private:
    QThread *thread;
    QTimer *timer;
    int fps;
    PacketRingBuffer *ringBuffer;
    QScreen *screen;
    TimeStamp *timstamp;


    QtAV::VideoEncoder *avVideoEnc;
    QtAV::VideoFormat *avVideoFormat;
    QtAV::VideoFrame *avVideoFrame;
};

#endif // SCREENCAPENCTHREAD_H
