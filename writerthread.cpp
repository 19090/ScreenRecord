#include "writerthread.h"
#include <QDebug>

WriterThread::WriterThread(PacketRingBuffer *_aupacketBuffer,
                           PacketRingBuffer *_avpacketBuffer,
                           QtAV::AudioEncoder *_auEnc,QtAV::VideoEncoder *_viEnc,QThread *parent) : QThread(parent)
{
    #ifdef USE_TWO_PACKET_BUF
    this->aupacketBuffer = _aupacketBuffer;
    this->avpacketBuffer = _avpacketBuffer;
#else
#endif
#ifndef SPLIT_TO_FILE
    muxer = new QtAV::AVMuxer;

    this->saveFileName = "./xx.mp4";
    muxer->setMedia(saveFileName);
    muxer->setFormat("mp4");
    muxer->copyProperties(_auEnc);
    muxer->copyProperties(_viEnc);

#else
    audiofile = "./_tmp.mp3";
    audioMuxer = new QtAV::AVMuxer;
    audioMuxer->setMedia(audiofile);
    audioMuxer->setFormat("mp3");
    audioMuxer->copyProperties(_auEnc);

    videofile = "./_tmp.h264";
    videoMuxer = new QtAV::AVMuxer;
    videoMuxer->setMedia(videofile);
    videoMuxer->setFormat("h264");
    videoMuxer->copyProperties(_viEnc);
#endif

    this->isRecordAudio = true;
    this->isRecordVideo = true;
}

void WriterThread::setRecordAudio(bool _enable)
{
    this->isRecordAudio = _enable;
}

void WriterThread::setRecordVideo(bool _enable)
{
    this->isRecordVideo = _enable;
}

WriterThread::WriterThread(PacketRingBuffer *_packetBuffer,
                           QtAV::AudioEncoder *_auEnc,
                           QtAV::VideoEncoder *_viEnc,
                           QThread *parent)
    : QThread(parent)
{
#ifdef USE_TWO_PACKET_BUF
    this->aupacketBuffer = _aupacketBuffer;
    this->avpacketBuffer = _avpacketBuffer;
#else
    this->packetBuffer = _packetBuffer;
#endif
#ifndef SPLIT_TO_FILE
    muxer = new QtAV::AVMuxer;

    this->saveFileName = "./xx.mp4";
    muxer->setMedia(saveFileName);
    muxer->setFormat("mp4");
    muxer->copyProperties(_auEnc);
    muxer->copyProperties(_viEnc);

#else
    audiofile = "./_tmp.mp3";
    audioMuxer = new QtAV::AVMuxer;
    audioMuxer->setMedia(audiofile);
    audioMuxer->setFormat("mp3");
    audioMuxer->copyProperties(_auEnc);

    videofile = "./_tmp.h264";
    videoMuxer = new QtAV::AVMuxer;
    videoMuxer->setMedia(videofile);
    videoMuxer->setFormat("h264");
    videoMuxer->copyProperties(_viEnc);
#endif
}
void WriterThread::setSaveFileName(const QString &_filename)
{
    if(_filename.isEmpty())
        return;

    this->saveFileName = _filename;
    muxer->setMedia(saveFileName);
}

bool WriterThread::start()
{
#ifndef SPLIT_TO_FILE
    if(muxer->open() == false)
        return false;
#else
    audioMuxer->open();
    videoMuxer->open();
#endif
    QThread::start(QThread::TimeCriticalPriority);
    return true;
}

bool WriterThread::stop()
{
    this->wait();
#ifndef SPLIT_TO_FILE
    muxer->close();
#else
    audioMuxer->close();
    videoMuxer->close();
#endif

    qDebug()<<"WriterThread::stop return";
    return true;
}

void WriterThread::run()
{
#ifdef USE_TWO_PACKET_BUF

    QtAV::Packet aupacket,avpacket;
    int avtype,autype;
#ifndef SPLIT_TO_FILE
    // bool auret,avret;
    msleep(1000/100);
    aupacketBuffer->getPacket(aupacket,autype,true);
    avpacketBuffer->getPacket(avpacket,avtype,true);

    while(1){
        if(avpacket.isEOF()){
            while(!aupacket.isEOF()){
                muxer->writeAudio(aupacket);
                aupacketBuffer->getPacket(aupacket,autype,true);
            }
            break;
        }

        if(aupacket.isEOF()){
            while(!avpacket.isEOF()){
                muxer->writeVideo(avpacket);
                avpacketBuffer->getPacket(avpacket,avtype,true);
            }
            break;
        }

        if(aupacket.pts < avpacket.pts){
//            qDebug()<<"write audio data tamp:"<<aupacket.pts;
            muxer->writeAudio(aupacket);
            aupacketBuffer->getPacket(aupacket,autype,true);
        }
        else {
//            qDebug()<<"write video data tamp:"<<avpacket.pts;
            muxer->writeVideo(avpacket);
            avpacketBuffer->getPacket(avpacket,avtype,true);
        }
    }
#else
    while(1){
        if(aupacketBuffer->getPacket(aupacket,autype,false))
        {
            //qDebug()<<"write audio data tamp:"<<aupacket.pts;
            audioMuxer->writeAudio(aupacket);
        }
        if(avpacketBuffer->getPacket(avpacket,avtype,false))
        {
            //qDebug()<<"write video data tamp:"<<avpacket.pts;
            videoMuxer->writeVideo(avpacket);
        }

        if(aupacket.isEOF()){
            while(!avpacket.isEOF()){
                if(avpacketBuffer->getPacket(avpacket,avtype,false))
                    videoMuxer->writeVideo(avpacket);
            }
            break;
        }

        if(avpacket.isEOF()){
            while(!aupacket.isEOF()){
                if(aupacketBuffer->getPacket(aupacket,autype,false))
                    audioMuxer->writeAudio(aupacket);
            }
            break;
        }

    }
#endif

#else
    QtAV::Packet packet;
    int type;
    packetBuffer->getPacket(packet,type,true);
    while(1){
        if(packet.isEOF()){
            break;
        }
        if(type == PACKET_TYPE_AUDIO){
            if(isRecordAudio)
            muxer->writeAudio(packet);
        }
        if(type == PACKET_TYPE_VIDEO){
            qDebug()<<"video packet size:"<<packet.data.size();
            if(isRecordVideo)
            muxer->writeVideo(packet);
        }
        packetBuffer->getPacket(packet,type,true);
    }
#endif
    qDebug()<<"write thread break while(1)";
}
