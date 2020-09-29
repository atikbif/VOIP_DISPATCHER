#include "udpworker.h"
#include <QThread>
#include <QDebug>
#include "checksum.h"

quint16 UDPWorker::id=0;

const quint8 UDPWorker::call_wav[] = {
                                      /*0x1E,*/0xF0,0x1C,0xA8,0x0C,0xA5,0xF6,0x92,0xE5,0x03,0xE0,0x63,0xEA,0xE9,0xFF,0x8A,0x14,0x63,0x1F,0x77,0x1B,0x77,0x0A,0x4E,0xF4,0x34,0xE3,0xC2,0xE0,0xD8,0xEC,0xDB,
                                      0x02,0x0B,0x16,0x40,0x1F,0xCB,0x1A,0x1A,0x07,0xE7,0xF1,0xE9,0xE2,0xAD,0xE1,0x7D,0xEE,0xEC,0x04,0x8A,0x17,0xFB,0x1F,0xEE,0x18,0x94,0x05,0x73,0xEF,0xB5,0xE1,0xC6,
                                      0xE2,0x52,0xF1,0x17,0x07,0x01,0x19,0x8F,0x1F,0xDE,0x16,0xE7,0x02,0xF6,0xED,0x9A,0xE1,0x0F,0xE3,0x57,0xF3,0x5A,0x09,0x6D,0x1A,0xFC,0x1F,0x9C,0x15,0x16,0x00,0x75,
                                      0xEB,0x9C,0xE0,0x88,0xE4,0x88,0xF5,0xB1,0x0B,0xCB,0x1C,0x3D,0x1F,0x27,0x13,0x24,0xFD,0xF4,0xE9,0xBF,0xE0,0x34,0xE5,0xE5,0xF8,0x19,0x0E,0x15,0x1D,0x52,0x1E,0x82,
                                      0x11,0x13,0xFB,0x75,0xE8,0x04,0xE0,0x11,0xE7,0x6B,0xFA,0x8C,0x10,0x4A,0x1E,0x39,0x1D,0xAD,0x0E,0xE8,0xF8,0xFE,0xE6,0x70,0xE0,0x21,0xE9,0x18,0xFD,0x09,0x12,0x65,
                                      0x1E,0xF0,0x1C,0xA8,0x0C,0xA5,0xF6,0x92,0xE5,0x03,0xE0,0x63,0xEA,0xE9,0xFF,0x89,0x14,0x63,0x1F,0x77,0x1B,0x77,0x0A,0x4E,0xF4,0x34,0xE3,0xC2,0xE0,0xD8,0xEC,0xDB,
                                      0x02,0x0B,0x16,0x40,0x1F,0xCB,0x1A,0x1A,0x07,0xE7,0xF1,0xE9,0xE2,0xAD,0xE1,0x7D,0xEE,0xEC,0x04,0x8A,0x17,0xFB,0x1F,0xEE,0x18,0x94,0x05,0x73,0xEF,0xB5,0xE1,0xC6,
                                      0xE2,0x52,0xF1,0x17,0x07,0x01,0x19,0x8F,0x1F,0xDE,0x16,0xE7,0x02,0xF6,0xED,0x9A,0xE1,0x0F,0xE3,0x57,0xF3,0x5A,0x09,0x6D,0x1A,0xFC,0x1F,0x9C,0x15,0x16,0x00,0x75,
                                      0xEB,0x9C,0xE0,0x88,0xE4,0x88,0xF5,0xB1,0x0B,0xCB,0x1C,0x3D,0x1F,0x27,0x13,0x24,0xFD,0xF4,0xE9,0xBF,0xE0,0x34,0xE5,0xE5,0xF8,0x19,0x0E,0x15,0x1D,0x52,0x1E,0x82,
                                      0x11,0x13,0xFB,0x75,0xE8,0x04,0xE0,0x11,0xE7,0x6B,0xFA,0x8C,0x10,0x4A,0x1E,0x39,0x1D,0xAD,0x0E,0xE8,0xF8,0xFE,0xE6,0x70,0xE0,0x21,0xE9,0x18,0xFD,0x09,0x12,0x65,
                                      0x1E,0xF0,0x1C,0xA8,0x0C,0xA5,0xF6,0x92,0xE5,0x03,0xE0,0x63,0xEA,0xE9,0xFF,0x89,0x14,0x63,0x1F,0x77,0x1B,0x77,0x0A,0x4E,0xF4,0x34,0xE3,0xC2,0xE0,0xD8,0xEC,0xDB,
                                      0x02,0x0B,0x16,0x40,0x1F,0xCB,0x1A,0x1A,0x07,0xE7,0xF1,0xE9,0xE2,0xAD,0xE1,0x7D,0xEE,0xEC,0x04,0x8A,0x17,0xFB,0x1F,0xEE,0x18,0x94,0x05,0x73,0xEF,0xB5,0xE1,0xC6,
                                      0xE2,0x52,0xF1,0x17,0x07,0x01,0x19,0x8F,0x1F,0xDE,0x16,0xE7,0x02,0xF6,0xED,0x9A,0xE1,0x0F,0xE3,0x57,0xF3,0x5A,0x09,0x6D,0x1A,0xFC,0x1F,0x9C,0x15,0x16,0x00,0x75,
                                      0xEB,0x9C,0xE0,0x88,0xE4,0x88,0xF5,0xB1,0x0B,0xCB,0x1C,0x3D,0x1F,0x27,0x13,0x24,0xFD,0xF4,0xE9,0xBF,0xE0,0x34,0xE5,0xE5,0xF8,0x19,0x0E,0x15,0x1D,0x52,0x1E,0x82,
                                      0x11,0x13,0xFB,0x75,0xE8,0x04,0xE0,0x11,0xE7,0x6B,0xFA,0x8C,0x10,0x4A,0x1E,0x39,0x1D,0xAD,0x0E,0xE8,0xF8,0xFE,0xE6,0x70,0xE0,0x21,0xE9,0x18,0xFD,0x09,0x12,0x65,0x1E
};

QByteArray UDPWorker::createRequestWriteAudio(const QByteArray &input, bool silentMode)
{
    QByteArray res;

    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    if(silentMode) res.append(0x02);else res.append(0x01); // cmd write audio
    if(pointId&0x80) {
        res.append(0x01);
        res.append(0xFF);
    }else if(grId&0x80) {
        res.append(grId & 0x7F);
        res.append('\0');
    }else {
        res.append(grId);  // group number
        res.append(pointId);   // point number
    }
    //qDebug() << grId << pointId;
    res.append(input);
    int crc = CheckSum::getCRC16(res);
    res.append(static_cast<char>(crc & 0xFF));
    res.append(static_cast<char>(crc >> 8));
    id++;
    return res;
}

QByteArray UDPWorker::createRequestSetVolume(quint8 group, quint8 point, quint8 value)
{
    QByteArray res;
    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    res.append(0x05); // cmd check link
    res.append(static_cast<char>(group));
    res.append(static_cast<char>(point));
    res.append(static_cast<char>(value));
    int crc = CheckSum::getCRC16(res);
    res.append(static_cast<char>(crc & 0xFF));
    res.append(static_cast<char>(crc >> 8));
    id++;
    return res;

}

QByteArray UDPWorker::createRequestSetInputFilter(quint8 group, quint8 point, quint8 filter, quint8 enableValue)
{
    QByteArray res;
    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    res.append(0x06); // cmd check link
    res.append(static_cast<char>(group));
    res.append(static_cast<char>(point));
    res.append(static_cast<char>(filter));
    res.append(static_cast<char>(enableValue));
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

    udp.write(createRequestCheckLink());
    if(udp.waitForReadyRead(wait_time_ms)) {
        static char receiveBuf[1024];
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
            if((quint8)receiveBuf[2]==0x04  && cnt==100*9+8)
            {
                QByteArray state;
                state.append(QByteArray::fromRawData(&receiveBuf[3],cnt-5));
                emit updateState(state);
            }
            else if((quint8)receiveBuf[2]==0x03  && cnt==165) {

                QByteArray state;
                state.append(QByteArray::fromRawData(&receiveBuf[3],cnt-5));
                emit updateGroupState(state);
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
            udp.disconnectFromHost();
            // попытка переподключения к контроллеру
            err_cnt = 0;
        }
    }
}

void UDPWorker::checkAudioCmd(QUdpSocket &udp)
{

    udp.write(createRequestCheckAudio());
    if(udp.waitForReadyRead(wait_time_ms)) {
        static char receiveBuf[1024];
        while(udp.hasPendingDatagrams()) {
            qint64 cnt = udp.readDatagram(receiveBuf,sizeof(receiveBuf));
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

void UDPWorker::setVolume(int group, int point, int value, bool allPoints)
{
    QMutexLocker locker(&mutex);
    volumeGroup = group;
    volumePoint = point;
    volumeValue = value;
    volumeCmd = true;
    volumeAll = allPoints;
    //qDebug() << group << point << value;
}

void UDPWorker::setInpConf(int group, int point, int filter, int enValue)
{
    QMutexLocker locker(&mutex);
    inpCfgCmd = true;
    inpGroup = group;
    inpPoint = point;
    inpFilter = filter;
    inpEn = enValue;
}

void UDPWorker::scan()
{
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
        bool workFlagState = workFlag;
        bool finishFlagstate = finishFlag;
        bool newAudioPacketFlagState = newAudioPacketFlag;
        bool checkAudioFlagState = checkAudioFlag;
        bool volumeCmdState = volumeCmd;
        bool volumeAllState = volumeAll;
        bool inpCfgCmdState = inpCfgCmd;
        mutex.unlock();
        if(workFlagState) {
            if(udp.state()==QUdpSocket::UnconnectedState) {
                udp.connectToHost(QHostAddress(ip),12145);
                //qDebug() << "TRY CONNECT" << ip;
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
            if(volumeCmdState) {
                if(volumeAllState) {
                    for(int i=0;i<volumePoint;i++) {
                        udp.write(createRequestSetVolume(static_cast<quint8>(volumeGroup),static_cast<quint8>(i+1),static_cast<quint8>(volumeValue)));
                        if(udp.waitForReadyRead(wait_time_ms)) {
                            qint64 cnt = 0;
                            while(udp.hasPendingDatagrams()) {
                                cnt = udp.readDatagram(receiveBuf,sizeof(receiveBuf));
                                if(cnt==0) break;
                            }
                        }
                        mutex.lock();
                        if(finishFlag) break;
                        mutex.unlock();
                        QThread::msleep(10);
                    }
                    mutex.lock();
                    volumeCmd = false;
                    volumeAll = false;
                    mutex.unlock();
                }else {
                    mutex.lock();
                    volumeCmd = false;
                    udp.write(createRequestSetVolume(static_cast<quint8>(volumeGroup),static_cast<quint8>(volumePoint),static_cast<quint8>(volumeValue)));
                    mutex.unlock();
                    if(udp.waitForReadyRead(wait_time_ms)) {
                        qint64 cnt = 0;
                        while(udp.hasPendingDatagrams()) {
                            cnt = udp.readDatagram(receiveBuf,sizeof(receiveBuf));
                            if(cnt==0) break;
                        }
                    }
                }

            }
            if(inpCfgCmdState) {
                mutex.lock();
                inpCfgCmd = false;
                udp.write(createRequestSetInputFilter(static_cast<quint8>(inpGroup),static_cast<quint8>(inpPoint),static_cast<quint8>(inpFilter),static_cast<quint8>(inpEn)));
                mutex.unlock();
                if(udp.waitForReadyRead(wait_time_ms)) {
                    qint64 cnt = 0;
                    while(udp.hasPendingDatagrams()) {
                        cnt = udp.readDatagram(receiveBuf,sizeof(receiveBuf));
                        if(cnt==0) break;
                    }
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
                  if(silent && (receiveBuf[2]==0x01 || receiveBuf[2]==0x02 || (quint8)receiveBuf[2]==0x82)) {
                      bool call_flag = (quint8)receiveBuf[2]==0x82?true:false;
                      int pckt_cnt = receiveBuf[5];
                      bool check_length = true;
                      std::vector<int> pckt_length;
                      if(pckt_cnt<=0 || pckt_cnt>5) check_length=false;
                      if(check_length) {
                          for(int i=0;i<pckt_cnt;i++) pckt_length.push_back((quint8)receiveBuf[6+i]);
                          check_length = false;
                          //for(int l:pckt_length) if(l) check_length=true;
                          if(std::any_of(pckt_length.begin(),pckt_length.end(),[](int i){return i!=0;})) check_length=true;
                      }

                      if(check_length) {
                          //if((quint8)receiveBuf[4]!=fromID  && ((quint8)receiveBuf[4]<100)) emit fromIDSignal((quint8)receiveBuf[4]);
                          //fromID = (quint8)receiveBuf[4];
                          //if(fromID>100) fromID = 0;
                          fromGroup = (quint8)receiveBuf[3];
                          fromPoint = (quint8)receiveBuf[4];
                          if(fromPoint>100) fromPoint = 0;
                          //qDebug() << grId << pointId << fromGroup << fromPoint;
                          //if(((grId&0x80) && (pointId&0x80)) || ((grId==fromGroup) && (pointId==fromPoint)))
                          {
                              //qDebug() << grId << pointId << fromGroup << fromPoint;
                          //if(fromID && (toID==0xFF || toID==fromID)) {
                              if(startFlag==false) {
                                  startFlag = true;
                                  emit startRecord((quint8)receiveBuf[3],(quint8)receiveBuf[4]);
                              }
                              if(startFlag) startTime = QDateTime::currentMSecsSinceEpoch();
                              int offset = 6+pckt_cnt;
                              decodeBufOffset = 0;
                              for(int i=0;i<pckt_cnt;i++) {
                                  if(call_flag) {
                                      //qDebug() << "CALL";
                                      QByteArray inp;
                                      for(int i=0;i<320;i++) {
                                          inp.append(static_cast<char>(call_wav[call_offset]));
                                          call_offset++;
                                          if(call_offset>=sizeof(call_wav)) call_offset=0;
                                      }
                                      emit updateAudio(inp);
                                  }else {
                                      //qDebug() << "NOT CALL";
                                      int frame_size = opus_decode(dec, (unsigned char *)&receiveBuf[offset], pckt_length.at(i), (opus_int16 *)&decodeBuf[decodeBufOffset], 1024, 0);
                                      offset+=pckt_length.at(i);
                                      if(frame_size==160) {
                                        QByteArray inp;
                                        inp.append(QByteArray::fromRawData(&decodeBuf[decodeBufOffset], frame_size * 2));
                                        decodeBufOffset += frame_size * 2;
                                        if(decodeBufOffset>=sizeof (decodeBuf)) decodeBufOffset=0;
                                        emit updateAudio(inp);
                                      }else {
                                          //QString arr;
                                          //for(int i=0;i<cnt;i++) arr+=QString::number((unsigned char)receiveBuf[i],16)+" ";
                                          //qDebug() << "!!!" << frame_size << i << pckt_cnt << pckt_length.at(i) << cnt << arr << fromID << toID;
                                      }
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


        QThread::msleep(1);
    }
}
