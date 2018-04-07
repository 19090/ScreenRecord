#include "writerthread.h"
#include <QDebug>

WriterThread::WriterThread(PacketRingBuffer *_aupacketBuffer,
                           PacketRingBuffer *_avpacketBuffer,
                           QtAV::AudioEncoder *_auEnc,QtAV::VideoEncoder *_viEnc,QThread *parent) : QThread(parent)
{
    this->aupacketBuffer = _aupacketBuffer;
    this->avpacketBuffer = _avpacketBuffer;
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
    muxer->open();
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
            //qDebug()<<"write audio data tamp:"<<aupacket.pts;
            muxer->writeAudio(aupacket);
            aupacketBuffer->getPacket(aupacket,autype,true);
        }
        else {
            //qDebug()<<"write video data tamp:"<<avpacket.pts;
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
    qDebug()<<"write thread break while(1)";
}
