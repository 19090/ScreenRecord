#include "auencthread.h"
#include <QDebug>

AuEncThread::AuEncThread(AuDataRingBuffer *_rawDataBuffer,PacketRingBuffer *_packetBuffer,QThread *parent) : QThread(parent)
{
    this->rawDataBuffer = _rawDataBuffer;
    this->packetBuffer = _packetBuffer;

    this->auEnc = QtAV::AudioEncoder::create("FFmpeg");
    avAudioFormat = new QtAV::AudioFormat;

    //sampleRate 8000 11025 22050 44100
    avAudioFormat->setSampleRate(44100);
    avAudioFormat->setChannels(1);
//    avAudioFormat->setChannelLayout(QtAV::AudioFormat::ChannelLayout_Stereo);
    avAudioFormat->setSampleFormat(QtAV::AudioFormat::SampleFormat_Signed16);
    auEnc->setAudioFormat(*avAudioFormat);
    auEnc->setBitRate(64000);//64k
    auEnc->setCodecName("libmp3lame"); //libmp3lame
    auEnc->setTimestampMode(QtAV::AVEncoder::TimestampCopy);
}

void AuEncThread::start()
{
    if(auEnc->open())
        qDebug()<<"audio enc open";
    else
        qDebug()<<"audio enc open false";
    QThread::start(QThread::TimeCriticalPriority);
}

void AuEncThread::run()
{
    QByteArray data;
    qreal timestamp;
    while(1){
        timestamp = 0;
        data.clear();

        rawDataBuffer->getData(data,timestamp);
//        qDebug()<<"data size:"<<data.size();
        if((data == "end") || (timestamp == 0.0))
            break;

        QtAV::AudioFrame frame(*avAudioFormat,data);
        if(!frame.isValid())
            continue;
        frame.setTimestamp(timestamp);
//        qDebug()<<frame.format();
        if(this->auEnc->encode(frame) == false){
            continue ;
        }
//         qDebug()<<auEnc->audioFormat();

        QtAV::Packet packet = this->auEnc->encoded();
        if(packet.isValid())
            packetBuffer->pushPacket(packet,PACKET_TYPE_AUDIO);
    }
    qDebug()<<"Audio encodec thread::run is break while(1)! ";

    while (auEnc->encode()) {
        QtAV::Packet packet = auEnc->encoded();
        if(packet.isValid())
            packetBuffer->pushPacket(packet,PACKET_TYPE_AUDIO);
    }

    auEnc->flush();
    auEnc->close();
    qDebug()<<"Audio encodec thread::run return";
}

bool AuEncThread::stop()
{
    this->wait();
    qDebug()<<"AuEncThread::wait return!";

    qDebug()<<"AuEncThread::stop write audio end data!";
    QtAV::Packet packet = QtAV::Packet::createEOF();
    packetBuffer->pushPacket(packet,PACKET_TYPE_AUDIO);
    return true;
}

QtAV::AudioEncoder *AuEncThread::getAudioEnc()
{
    return this->auEnc;
}

