#include "screencapencthread.h"
#include <QApplication>
#include <QDebug>


ScreenCapEncThread::ScreenCapEncThread(TimeStamp *_timstamp,PacketRingBuffer *_ringBuffer,QObject *parent) : QObject(parent)
{
    thread = new QThread;

    this->fps = 10;
    this->ringBuffer = _ringBuffer;
    screen = qApp->screens()[0];
    this->timstamp = _timstamp;

    avVideoEnc = QtAV::VideoEncoder::create("FFmpeg");
    avVideoEnc->setBitRate(1400000);
    avVideoEnc->setFrameRate(fps);
    avVideoEnc->setWidth(1024);
    avVideoEnc->setHeight(768);
    avVideoEnc->setPixelFormat(QtAV::VideoFormat::Format_YUV420P);
    avVideoEnc->setCodecName("libx264");//"libx264");

    //视频pts自动增长，与实际的time stamp不同 要选择copy
    avVideoEnc->setTimestampMode(QtAV::AVEncoder::TimestampMonotonic);


    this->moveToThread(thread);


}

bool ScreenCapEncThread::start()
{
    avVideoEnc->open();
    thread->start();
    timer = new QTimer(0);
    timer->setInterval(1000/fps);
    connect(timer, SIGNAL(timeout()),
            this, SLOT(slot_timerTimeout()), Qt::DirectConnection);

    timer->start();
    timer->moveToThread(thread);
    return true;
}

bool ScreenCapEncThread::stop()
{
    timer->stop();
    thread->quit();
    thread->wait();
    while(avVideoEnc->encode()){
        QtAV::Packet packet = avVideoEnc->encoded();
        if(packet.isValid()){
          ringBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
        }
    }
    avVideoEnc->flush();
    avVideoEnc->close();

    return true;
}

void ScreenCapEncThread::slot_timerTimeout()
{
    qDebug()<<"ScreenCapEncThread::slot_timerTimeout";
    QPixmap pixmap = screen->grabWindow(QApplication::desktop()->winId(),
                                        8, 8,1024,768);

    QtAV::VideoFrame frame(pixmap.toImage());
    frame = frame.to(QtAV::VideoFormat::Format_YUV420P);


    qreal _timest = this->timstamp->getTimeStamp() / 1000000.0 ;
    frame.setTimestamp(_timest);
    avVideoEnc->encode(frame);
    QtAV::Packet packet = avVideoEnc->encoded();
    if(packet.isValid()){
        ringBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
    }
}

QtAV::VideoEncoder *ScreenCapEncThread::getVideoEnc()
{
    return this->avVideoEnc;
}
