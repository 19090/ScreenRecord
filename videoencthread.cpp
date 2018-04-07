#include "videoencthread.h"
#include <QDebug>

VideoEncThread::VideoEncThread(VideoRingBuffer *_videoBuffer,PacketRingBuffer *_packetBuffer,QThread *parent) : QThread(parent)
{
    this->videoBuffer = _videoBuffer;
    this->packetBuffer = _packetBuffer;

    this->fps = 10;
    avVideoEnc = QtAV::VideoEncoder::create("FFmpeg");
    avVideoEnc->setBitRate(400000);
    avVideoEnc->setFrameRate(fps);
    avVideoEnc->setWidth(1280);
    avVideoEnc->setHeight(720);
    avVideoEnc->setPixelFormat(QtAV::VideoFormat::Format_YUV420P);
    avVideoEnc->setCodecName("libx264");//"libx264");mpeg4
    avVideoEnc->setTimestampMode(QtAV::AVEncoder::TimestampMonotonic);

    this->avctx = (AVCodecContext *)avVideoEnc->codecContext();
    //av_opt_set((void *)avctx->priv_data,"preset","ultrafast",0);
    this->isExited = false;

   // qDebug()<<QtAV::VideoEncoder::supportedCodecs();
}
void VideoEncThread::start()
{
    avVideoEnc->open();
    QThread::start(QThread::NormalPriority);
}

void VideoEncThread::run()
{
    QImage image;
    qreal timestamp;
    while(!isExited){
        timestamp = 0;

        if(videoBuffer->getData(image,timestamp)==false)
            continue ;
        //qDebug()<<"encodec thread "<<timestamp;
//        qDebug()<<"VideoEncThread::run encode";
        QtAV::VideoFrame frame(image);
        frame = frame.to(QtAV::VideoFormat::Format_YUV420P);
        if(frame.isValid())
            frame.setTimestamp(timestamp);

        if(avVideoEnc->encode(frame) == false){
            continue ;
        }

        QtAV::Packet packet = avVideoEnc->encoded();
        if(packet.isValid()){
            packetBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
            //qDebug()<<"####"<<packet.dts;
        }
    }
}

bool VideoEncThread::stop()
{
    this->isExited = true;

    this->wait();
    qDebug()<<"VideoEncThread::stop return!";
    return true;
}

QtAV::VideoEncoder *VideoEncThread::getVideoEnc()
{
    return this->avVideoEnc;
}
