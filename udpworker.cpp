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
    QByteArray res;
    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    res.append(0xD0); // cmd check link
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
    udp.write(createRequestReadState());
    if(udp.waitForReadyRead(wait_time_ms)) {
        qint64 cnt = 0;
        while(udp.hasPendingDatagrams()) {
            cnt = udp.readDatagram(receiveBuf,sizeof(receiveBuf));
            if(cnt==0) break;
        }
        if(cnt>0) {
            if(cnt>=100 && (quint8)receiveBuf[2]==0xD0) emit updateState(QByteArray::fromRawData(&receiveBuf[3],cnt-5));
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

UDPWorker::UDPWorker(QObject *parent) : QObject(parent)
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

    static int tmp=0;

    QUdpSocket udp;
    udp.connectToHost(QHostAddress("192.168.5.85"),12145);
    for(;;) {
        mutex.lock();
        workFlagState = workFlag;
        finishFlagstate = finishFlag;
        newAudioPacketFlagState = newAudioPacketFlag;
        checkAudioFlagState = checkAudioFlag;
        mutex.unlock();
        if(workFlagState) {
            check_cnt++;
            if(check_cnt>=30) {
                check_cnt = 0;
                //checkLink(udp);

                if(checkAudioFlagState) {
                    checkAudioFlagState = false;
                    mutex.lock();
                    checkAudioFlag = false;
                    mutex.unlock();
                    checkAudioCmd(udp);
                }else {
                    readState(udp);
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
                         // qDebug() << "P:" << pckt_cnt << pckt_length << QByteArray::fromRawData(receiveBuf,100);
                      }

                  }
                  /*int enc_size1 = receiveBuf[6];
                  int enc_size2 = receiveBuf[7];
                  decodeBufOffset = 0;
                  auto frame_size = opus_decode(dec, (unsigned char *)&receiveBuf[8], enc_size1, (opus_int16 *)&decodeBuf[decodeBufOffset], 1024, 0);
                  if(frame_size>0) {
                    decodeBufOffset += frame_size * 2;
                    QByteArray inp = QByteArray::fromRawData(&decodeBuf[0], frame_size * 2);
                    emit updateAudio(inp);
                  }

                  qDebug() << "DECODE1" << enc_size1 << frame_size;
                  frame_size = opus_decode(dec, (unsigned char *)&receiveBuf[8+enc_size1], enc_size2, (opus_int16 *)&decodeBuf[decodeBufOffset], 1024, 0);
                  if(frame_size>0) {
                    QByteArray inp = QByteArray::fromRawData(&decodeBuf[decodeBufOffset], frame_size * 2);
                    emit updateAudio(inp);
                  }
                  qDebug() << "DECODE2" << enc_size2 << frame_size;*/
                }
                /*char ptr[1024];
                int cnt=0;
                for(auto ch:packet) ptr[cnt++]=ch;
                int offset=0;*/

                  //auto frame_size = opus_decode(dec, (unsigned char*)&ptr[0], cnt, (opus_int16*)&decodeBuf[0], 1024, 0);
                  //QByteArray inp = QByteArray::fromRawData(&decodeBuf[0],frame_size*2);
                  //emit updateAudio(inp);




                  //qDebug() << "DECODE" << frame_size;
                  /*speex_bits_read_from(&bits, &ptr[offset], 20);
                  speex_decode_int(dec_state, &bits, (spx_int16_t*)&decodeBuf[decodeBufOffset]);
                  QByteArray inp = QByteArray::fromRawData(&decodeBuf[decodeBufOffset],320);
                  decodeBufOffset+=320;
                  emit updateAudio(inp);*/
                /*mutex.lock();
                newAudioPacketFlag=0;
                udp.write(createRequestWriteAudio(packet,silent));
                mutex.unlock();
                if(udp.waitForReadyRead(wait_time_ms)) {
                  qint64 cnt = 0;
                  while (udp.hasPendingDatagrams()) {
                    cnt = udp.readDatagram(receiveBuf, sizeof(receiveBuf));
                    if (cnt==0) break;
                  }
                  if(cnt>6+2) {
                    cnt-=8;
                    int offset = 6;
                    if(receiveBuf[5]!=fromID) emit fromIDSignal(receiveBuf[5]);
                    fromID = receiveBuf[5];
                    decodeBufOffset = 0;
                    if(toID==0xFF || toID==fromID)
                    while(cnt>=20) {
                      speex_bits_read_from(&bits, &receiveBuf[offset], 20);
                      speex_decode_int(dec_state, &bits, (spx_int16_t*)&decodeBuf[decodeBufOffset]);
                      QByteArray inp = QByteArray::fromRawData(&decodeBuf[decodeBufOffset],320);
                      decodeBufOffset+=320;
                      emit updateAudio(inp);
                      cnt-=20;
                      offset+=20;
                    }
                  }
              }*/
           }
        }
        if(finishFlagstate) break;
        QThread::msleep(1);

    }
}
