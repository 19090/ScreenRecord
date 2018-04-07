#ifndef PACKETRINGBUFFER_H
#define PACKETRINGBUFFER_H

#include <QSemaphore>
#include <QMutex>
#include <QList>
#include <Qtav/Packet.h>
//#define BUF_COUNT 5

/*
 * audio enc Packet 数据的环形缓冲区
*/
#define PACKET_TYPE_VIDEO 0x11
#define PACKET_TYPE_AUDIO 0x22

struct PacketData {
    QtAV::Packet packet;
    int type;
};

class PacketRingBuffer
{
public:
   PacketRingBuffer();

    bool pushPacket(const QtAV::Packet &_packet,const int &_type);//push不了会休眠
    bool getPacket(QtAV::Packet &_packet,int &_type,bool _wait);//get不了会休眠
    void clear()
    {
        delete freeCount;
        delete usedCount;
        freeCount = new QSemaphore(25);
        usedCount = new QSemaphore(0);
        this->ringBuffer.clear();
    }
private:
    QList<PacketData> ringBuffer;
    QSemaphore *freeCount;
    QSemaphore *usedCount;
    QMutex mutex;

};

#endif // PACKETRINGBUFFER_H
