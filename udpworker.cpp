#include "udpworker.h"
#include <QThread>
#include <QDebug>
#include "checksum.h"

quint16 UDPWorker::id=0;

QByteArray UDPWorker::createRequestWriteAudio(const QByteArray &input, bool silentMode)
{
    QByteArray res;

    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    if(silentMode) res.append(0x02);else res.append(0x01); // cmd write audio
    res.append(0x01);  // group number
    res.append(toID);   // point number
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

QByteArray UDPWorker::createRequestReadState()
{
    static int state=0;
    QByteArray res;
    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    //res.append(0xD0); // cmd check link
    switch(state) {
        case 0:
            res.append(0x04);
            res.append('\0');
            break;
        case 1:
            res.append(0x03);
        break;
    }
    state++;
    if(state>=2) state=0;
    int crc = CheckSum::getCRC16(res);
    res.append(static_cast<char>(crc & 0xFF));
    res.append(static_cast<char>(crc >> 8));
    id++;
    return res;
}

QByteArray UDPWorker::createRequestCheckAudio()
{
    QByteArray res;
    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    res.append(0xD1); // cmd check link
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

void UDPWorker::readState(QUdpSocket &udp)
{
    static quint16 err_cnt=0;
    static char receiveBuf[1024];
    //static qint64 t=0;
    udp.write(createRequestReadState());
    if(udp.waitForReadyRead(wait_time_ms)) {
        qint64 cnt = 0;
        while(udp.hasPendingDatagrams()) {
            cnt = udp.readDatagram(receiveBuf,sizeof(receiveBuf));
            if(cnt==0) break;
        }
        if(cnt>0) {
            //if(cnt>=100 && (quint8)receiveBuf[2]==0xD0) emit updateState(QByteArray::fromRawData(&receiveBuf[3],cnt-5));
            if((quint8)receiveBuf[2]==0x04)
            {
                if(cnt>=100) {
                    emit updateState(QByteArray::fromRawData(&receiveBuf[3],cnt-5));
                }
            }
            if((quint8)receiveBuf[2]==0x03) {

                emit updateGroupState(QByteArray::fromRawData(&receiveBuf[3],cnt-5));
                //qDebug() << QDateTime::currentMSecsSinceEpoch()-t;
                //t= QDateTime::currentMSecsSinceEpoch();
            }

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

void UDPWorker::checkAudioCmd(QUdpSocket &udp)
{
    static char receiveBuf[1024];
    udp.write(createRequestCheckAudio());
    if(udp.waitForReadyRead(wait_time_ms)) {
        qint64 cnt = 0;
        while(udp.hasPendingDatagrams()) {
            cnt = udp.readDatagram(receiveBuf,sizeof(receiveBuf));
            if(cnt==0) break;
        }
    }
}

UDPWorker::UDPWorker(const QString &ip, QObject *parent) : ip(ip), QObject(parent)
{
  //int tmp = 0;
  int error = 0;
  enc = opus_encoder_create(8000, 1, OPUS_APPLICATION_VOIP, &error);
  opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(1));
  opus_encoder_ctl(enc, OPUS_SET_BITRATE(8000));
  dec = opus_decoder_create(8000, 1, &error);
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

void UDPWorker::checkAudio()
{
    QMutexLocker locker(&mutex);
    checkAudioFlag = true;
}

void UDPWorker::scan()
{
    bool workFlagState = false;
    bool finishFlagstate = false;
    bool newAudioPacketFlagState = false;
    bool checkAudioFlagState = false;
    int check_cnt = 0;
    static char receiveBuf[1024];
    static char decodeBuf[3000];
    static int decodeBufOffset = 0;

    static qint64 t = 0;


    static int tmp=0;

    QUdpSocket udp;
    //udp.connectToHost(QHostAddress("192.168.5.85"),12145);
    //udp.connectToHost(QHostAddress(ip),12145);
    //QThread::msleep(100);
    //connect(udp,&QUdpSocket::hostFound,[](){});

    for (;;) {
        mutex.lock();
        workFlagState = workFlag;
        finishFlagstate = finishFlag;
        newAudioPacketFlagState = newAudioPacketFlag;
        checkAudioFlagState = checkAudioFlag;
        mutex.unlock();
        if(workFlagState) {
            if(udp.state()==QUdpSocket::UnconnectedState) {
                udp.connectToHost(QHostAddress(ip),12145);
                qDebug() << "TRY CONNECT" << ip;
                //QThread::msleep(100);
            }
            if(QDateTime::currentMSecsSinceEpoch()-t>=100) {
                t = QDateTime::currentMSecsSinceEpoch();
                if(checkAudioFlagState) {
                    checkAudioFlagState = false;
                    mutex.lock();
                    checkAudioFlag = false;
                    mutex.unlock();
                    checkAudioCmd(udp);
                }else {
                    readState(udp);
                    //qDebug() << QDateTime::currentMSecsSinceEpoch()-t;
                }
            }
            if(newAudioPacketFlagState) {
                mutex.lock();
                newAudioPacketFlag=0;
                udp.write(createRequestWriteAudio(packet,silent));
                mutex.unlock();
                if (udp.waitForReadyRead(wait_time_ms)) {
                  qint64 cnt = 0;
                  while (udp.hasPendingDatagrams()) {
                    cnt = udp.readDatagram(receiveBuf, sizeof(receiveBuf));
                    if (cnt==0) break;
                  }
                  if(silent && (receiveBuf[2]==0x01 || receiveBuf[2]==0x02)) {
                      int pckt_cnt = receiveBuf[5];
                      bool check_length = true;
                      std::vector<int> pckt_length;
                      if(pckt_cnt<=0 || pckt_cnt>5) check_length=false;
                      if(check_length) {
                          for(int i=0;i<pckt_cnt;i++) pckt_length.push_back((quint8)receiveBuf[6+i]);
                          check_length = false;
                          for(int l:pckt_length) if(l) check_length=true;
                      }

                      if(check_length) {
                          if((quint8)receiveBuf[4]!=fromID  && ((quint8)receiveBuf[4]<100)) emit fromIDSignal((quint8)receiveBuf[4]);
                          fromID = (quint8)receiveBuf[4];
                          if(fromID>100) fromID = 0;
                          //qDebug() << fromID;
                          if(fromID && (toID==0xFF || toID==fromID)) {
                              if(startFlag==false) {
                                  startFlag = true;
                                  emit startRecord((quint8)receiveBuf[3],(quint8)receiveBuf[4]);
                              }
                              if(startFlag) startTime = QDateTime::currentMSecsSinceEpoch();
                              int offset = 6+pckt_cnt;
                              decodeBufOffset = 0;
                              int frame_size=0;
                              for(int i=0;i<pckt_cnt;i++) {
                                  frame_size = opus_decode(dec, (unsigned char *)&receiveBuf[offset], pckt_length.at(i), (opus_int16 *)&decodeBuf[decodeBufOffset], 1024, 0);
                                  offset+=pckt_length.at(i);
                                  if(frame_size==160) {
                                    QByteArray inp = QByteArray::fromRawData(&decodeBuf[decodeBufOffset], frame_size * 2);
                                    decodeBufOffset += frame_size * 2;
                                    if(decodeBufOffset>=sizeof (decodeBuf)) decodeBufOffset=0;
                                    emit updateAudio(inp);
                                  }else {
                                      //QString arr;
                                      //for(int i=0;i<cnt;i++) arr+=QString::number((unsigned char)receiveBuf[i],16)+" ";
                                      //qDebug() << "!!!" << frame_size << i << pckt_cnt << pckt_length.at(i) << cnt << arr << fromID << toID;
                                  }
                              }
                              //qDebug() << "P:" << pckt_cnt << pckt_length;
                          }
                      }else {
                          fromID = 0;
                          emit fromIDSignal(0);
                         // qDebug() << "P:" << pckt_cnt << pckt_length << QByteArray::fromRawData(receiveBuf,100);
                      }
                  }
               }
           }
        }else {
            if(udp.state()!=QUdpSocket::UnconnectedState) udp.disconnectFromHost();
        }
        if(startFlag) {
            if(QDateTime::currentMSecsSinceEpoch()-startTime>=1000) {
                startFlag = false;
                emit stopRecord();
            }
        }
        if(finishFlagstate) break;

        /**/
        QThread::msleep(1);
    }
}
