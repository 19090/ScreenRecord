#ifndef VIDEOENCTHREAD_H
#define VIDEOENCTHREAD_H

#include <QObject>
#include <QThread>
#include <QtAV/VideoEncoder.h>
#include <QtAV/VideoFormat.h>
#include <QtAV/VideoFrame.h>
#include <QtAV/Filter.h>

#include "videoringbuffer.h"
#include "packetringbuffer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
class VideoEncThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoEncThread(VideoRingBuffer *_videoBuffer,PacketRingBuffer *_packetBuffer,QThread *parent = nullptr);
    QtAV::VideoEncoder *getVideoEnc();
    void setScreenSize(const QSize &size)
    {
        avVideoEnc->setWidth(size.width());
        avVideoEnc->setHeight(size.height());
    }
    void setScreenSize(int w,int h)
    {
        avVideoEnc->setWidth(w);
        avVideoEnc->setHeight(h);
    }
signals:

protected:
    void run();

public slots:
    void start();
    bool stop();
private:
    QtAV::VideoEncoder *avVideoEnc;
    QtAV::VideoFormat *avVideoFormat;
    QtAV::VideoFrame *avVideoFrame;

    VideoRingBuffer *videoBuffer;
    PacketRingBuffer *packetBuffer;

    bool isExited;

    int fps;

    AVCodecContext *avctx;
};

#endif // VIDEOENCTHREAD_H
