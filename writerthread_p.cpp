#include "writerthread.h"
#include <QDebug>


WriterThread::WriterThread(QThread *parent) : QThread(parent)
{
    muxer = new QtAV::AVMuxer;

    this->saveFileName = "./temp.mp4";
    muxer->setMedia(saveFileName);
    muxer->setFormat("mp4");

    audioPacketBuffer = NULL;
    videoPacketBuffer = NULL;

    audioEncoder = NULL;
    videoEncoder = NULL;
}

void WriterThread::setEncoder(QtAV::AudioEncoder *_auEnc, QtAV::VideoEncoder *_viEnc)
{
    if(_auEnc != NULL){
        this->audioEncoder = _auEnc;
        muxer->copyProperties(_auEnc);
    }
    if(_viEnc != NULL){
        this->videoEncoder = _viEnc;
        muxer->copyProperties(_viEnc);
    }
}

void WriterThread::setPacketBuffer(PacketRingBuffer *_aupacketBuffer, PacketRingBuffer *_avpacketBuffer)
{
    this->audioPacketBuffer = _aupacketBuffer;
    this->videoPacketBuffer = _avpacketBuffer;
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
    if((audioEncoder != NULL)
        &&(audioPacketBuffer == NULL)){
        qDebug()<<"audio encoder success,audio packet buffer no set!";
        return false;
    }
    if((videoEncoder != NULL)
            &&(videoPacketBuffer == NULL)){
        qDebug()<<"video encoder success,video packet buffer no set!";
        return false;
    }

    if((audioEncoder== NULL)&&(videoEncoder==NULL)){
        return false;
    }

    if((audioPacketBuffer==NULL)&&(videoPacketBuffer == NULL))
        return false;

    if(muxer->open() == false)
        return false;

    QThread::start(QThread::HighestPriority);
    return true;
}

bool WriterThread::stop()
{
    this->wait();
    muxer->close();
    qDebug()<<"WriterThread::stop return";
    return true;
}

void WriterThread::run()
{
    QtAV::Packet aupacket,avpacket;
    int avtype,autype;
    // bool auret,avret;
    msleep(1000/100);
    if(audioEncoder == NULL){
        videoPacketBuffer->getPacket(avpacket,avtype,true);    
        while(!avpacket.isEOF()){
            muxer->writeVideo(avpacket);
            videoPacketBuffer->getPacket(avpacket,avtype,true);
        }
    }
    else if(videoEncoder == NULL){
        audioPacketBuffer->getPacket(aupacket,autype,true);
        while(!aupacket.isEOF()){
            muxer->writeAudio(aupacket);
            audioPacketBuffer->getPacket(aupacket,autype,true);
        }
    }
    else {
        audioPacketBuffer->getPacket(aupacket,autype,true);
        videoPacketBuffer->getPacket(avpacket,avtype,true);
        while(1){
            if(avpacket.isEOF()){
                while(!aupacket.isEOF()){
                    muxer->writeAudio(aupacket);
                    audioPacketBuffer->getPacket(aupacket,autype,true);
                }
                break;
            }

            if(aupacket.isEOF()){
                while(!avpacket.isEOF()){
                    muxer->writeVideo(avpacket);
                    videoPacketBuffer->getPacket(avpacket,avtype,true);
                }
                break;
            }

            if(aupacket.pts < avpacket.pts){
                muxer->writeAudio(aupacket);
                audioPacketBuffer->getPacket(aupacket,autype,true);
            }
            else {
                muxer->writeVideo(avpacket);
                videoPacketBuffer->getPacket(avpacket,avtype,true);
            }
        }
    }
    qDebug()<<"write thread break while(1)";
}
