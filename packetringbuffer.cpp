#include "packetringbuffer.h"
#include <QDebug>

PacketRingBuffer::PacketRingBuffer()
{
    freeCount = new QSemaphore(25);
    usedCount = new QSemaphore(0);
    ringBuffer.clear();
}

bool PacketRingBuffer::pushPacket(const QtAV::Packet &_packet,const int &_type)
{
    freeCount->acquire(1);
//    if(_type == PACKET_TYPE_AUDIO){
//        qDebug()<<"push packet audio pts:"<<_packet.pts
//               <<" dts:"<<_packet.dts<<"duration:"<<_packet.duration;
//    }
//    else {
//        qDebug()<<"push packet video pts:"<<_packet.pts
//               <<" dts:"<<_packet.dts<<"duration:"<<_packet.duration;
//    }
    mutex.lock();
    PacketData data;
    data.packet = _packet;
    data.type = _type;
    ringBuffer.append(data);
    mutex.unlock();

    usedCount->release(1);
    return true;
}

bool PacketRingBuffer::getPacket(QtAV::Packet &_packet,int &_type,bool _wait)
{
    if(_wait){
        usedCount->acquire(1);
    }
    else{
        if(usedCount->tryAcquire(1) == false)
            return false;
    }

    mutex.lock();
    PacketData data;
    data = ringBuffer.takeFirst();
    _packet = data.packet;
    _type = data.type;

    mutex.unlock();

    freeCount->release(1);
    return true;
}
