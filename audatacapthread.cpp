#include "audatacapthread.h"
#include <QDebug>

AuDataCapThread::AuDataCapThread(TimeStamp *_timestamp,AuDataRingBuffer *_rawRingBuffer,QObject *parent) : QObject(parent)
{
    this->timestamp = _timestamp;
    this->rawRingBuffer = _rawRingBuffer;

    thread = new QThread;

    qtformat.setSampleRate(44100);
    qtformat.setChannelCount(1);
    qtformat.setSampleSize(16);
    qtformat.setCodec("audio/pcm");
    qtformat.setByteOrder(QAudioFormat::LittleEndian);
    qtformat.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(qtformat)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        qtformat = info.nearestFormat(qtformat);
    }

    qtaudioinput = new QAudioInput(qtformat);
    qtaudioinput->setBufferSize(11520);
    qtaudioinput->setVolume(0.5);
    qDebug()<<"##########audio sample rate :"<<qtaudioinput->format().sampleRate();
    qDebug()<<"##########audio buffer size :"<<qtaudioinput->bufferSize();
    qtaudioinput->moveToThread(thread);
    this->moveToThread(thread);
}


bool AuDataCapThread::start()
{
    this->lastTimestamp = 0;

    this->qtaudiodev = qtaudioinput->start();
    this->qtaudiodev->moveToThread(thread);
    connect(qtaudiodev,SIGNAL(readyRead()),this,SLOT(slot_pushData2Buf()));
    thread->start(QThread::NormalPriority);
    return true;
}

void AuDataCapThread::slot_pushData2Buf()
{
    qint64 _timestamp = qtaudioinput->elapsedUSecs();
    qreal _setstamp = _timestamp/1000000.0;

    QByteArray data = qtaudiodev->readAll();

    if(data.isEmpty())
        return ;

    if(data.size() > (11520/5)){
        int count = data.size()/(11520/5);
        for(int i=0;i<count;i++){
            QByteArray _tmpdata = data.mid(i*(11520/5),11520/5);
            qreal _tmpstamp = (lastTimestamp+(_timestamp-lastTimestamp)/count*(i+1))/1000000.0;
            timestamp->setTimeStamp(_tmpstamp);  //去掉
            rawRingBuffer->pushData(_tmpdata,_tmpstamp);
            //qDebug()<<"audio time stamp:"<<_tmpstamp;
        }
    }
    else{
       // qDebug()<<"audio time stamp:"<<_setstamp;
        timestamp->setTimeStamp(_setstamp);  //去掉
        rawRingBuffer->pushData(data,_setstamp);
    }
    lastTimestamp = _timestamp;


}

bool AuDataCapThread::stop()
{
    disconnect(qtaudiodev,SIGNAL(readyRead()),this,SLOT(slot_pushData2Buf()));
    qtaudioinput->stop();
    thread->quit();
    thread->wait();

    //写入end数据到 audio raw buffer;
    qDebug()<<"audio capture thread push end data";
    QByteArray data = "end";
    rawRingBuffer->pushData(data,0.0);

    return true;
}

bool AuDataCapThread::setAudioFmt(const QAudioFormat &fmt)
{
    if(qtaudioinput->state() != QAudio::StoppedState)
        return false;

    this->qtformat = fmt;

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(qtformat)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        qtformat = info.nearestFormat(qtformat);
    }

    delete qtaudioinput;
    qtaudioinput = new QAudioInput(qtformat);
    qtaudioinput->setBufferSize(11520);
    qtaudioinput->setVolume(0.5);
    qtaudioinput->moveToThread(thread);

    return true;
}

QAudioFormat AuDataCapThread::getAudioFmt()
{
    return qtformat;
}


