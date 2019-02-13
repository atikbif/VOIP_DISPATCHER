#include "udpworker.h"
#include <QThread>
#include <QDebug>
#include "checksum.h"

quint16 UDPWorker::id=0;

QByteArray UDPWorker::createRequestWriteAudio(const QByteArray &input)
{
    QByteArray res;
    quint16 cnt = static_cast<quint16>(input.count());
    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    res.append(0x01); // cmd write audio
    res.append(static_cast<char>(cnt>>8)); // data length
    res.append(static_cast<char>(cnt&0xFF));
    res.append(input);
    int crc = CheckSum::getCRC16(res);
    res.append(static_cast<char>(crc & 0xFF));
    res.append(static_cast<char>(crc >> 8));
    id++;
    return res;
}

QByteArray UDPWorker::createRequestCheckLink()
{
    QByteArray res;
    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    res.append(0xA0); // cmd check link
    int crc = CheckSum::getCRC16(res);
    res.append(static_cast<char>(crc & 0xFF));
    res.append(static_cast<char>(crc >> 8));
    id++;
    return res;
}

void UDPWorker::checkLink(QUdpSocket &udp)
{
    static quint16 err_cnt=0;
    static char receiveBuf[1024];
    udp.write(createRequestCheckLink());
    if(udp.waitForReadyRead(wait_time_ms)) {
        qint64 cnt = 0;
        while(udp.hasPendingDatagrams()) {
            cnt = udp.readDatagram(receiveBuf,sizeof(receiveBuf));
            if(cnt==0) break;
        }
        if(cnt>0) {
            err_cnt=0;
            mutex.lock();
            if(linkState==false) emit linkStateChanged(true);
            linkState = true;
            mutex.unlock();
        }
    }else {
        err_cnt++;
        if(err_cnt>=5) {
            mutex.lock();
            if(linkState==true) emit linkStateChanged(false);
            linkState = false;
            mutex.unlock();
            err_cnt = 0;
        }
    }
}

UDPWorker::UDPWorker(QObject *parent) : QObject(parent)
{

}

void UDPWorker::start()
{
     QMutexLocker locker(&mutex);
     workFlag = true;
     linkState = false;
}

void UDPWorker::stop()
{
     QMutexLocker locker(&mutex);
     workFlag = false;
}

void UDPWorker::finish()
{
    QMutexLocker locker(&mutex);
    finishFlag = true;
}

void UDPWorker::writeAudioPacket(const QByteArray &input)
{
    QMutexLocker locker(&mutex);
    newAudioPacketFlag = true;
    packet = input;
}

bool UDPWorker::getLinkState() const
{
    QMutexLocker locker(&mutex);
    return linkState;
}

void UDPWorker::scan()
{
    bool workFlagState = false;
    bool finishFlagstate = false;
    bool newAudioPacketFlagState = false;
    int check_cnt = 0;


    QUdpSocket udp;
    udp.connectToHost(QHostAddress("192.168.5.85"),7);
    for(;;) {
        mutex.lock();
        workFlagState = workFlag;
        finishFlagstate = finishFlag;
        newAudioPacketFlagState = newAudioPacketFlag;
        mutex.unlock();
        if(workFlagState) {
            check_cnt++;
            if(check_cnt>=100) {
                check_cnt = 0;
                checkLink(udp);
            }
            if(newAudioPacketFlagState) {
                mutex.lock();
                newAudioPacketFlag=0;
                udp.write(createRequestWriteAudio(packet));
                mutex.unlock();
                udp.waitForBytesWritten(10);
            }
        }
        if(finishFlagstate) break;
        QThread::msleep(1);

    }
}
