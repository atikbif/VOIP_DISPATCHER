#include "udpworker.h"
#include <QThread>
#include <QDebug>
#include "checksum.h"

quint16 UDPWorker::id=0;

QByteArray UDPWorker::createRequestWriteAudio(const QByteArray &input, bool silentMode)
{
    QByteArray res;
    quint16 cnt = static_cast<quint16>(input.count());
    res.append(static_cast<char>(id>>8));
    res.append(static_cast<char>(id&0xFF));
    if(silentMode) res.append(0x02);else res.append(0x01); // cmd write audio
    res.append(static_cast<char>(cnt>>8)); // data length
    res.append(static_cast<char>(cnt&0xFF));
    res.append(toID);
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
  int tmp = 0;
  state = speex_encoder_init(&speex_nb_mode);
  speex_bits_init(&bits);
  tmp = 4;
  speex_encoder_ctl(state, SPEEX_SET_QUALITY, &tmp);
  tmp = 8000;
  speex_encoder_ctl(state, SPEEX_SET_SAMPLING_RATE, &tmp);
    quint32 enc_frame_size;
    speex_encoder_ctl(state, SPEEX_GET_FRAME_SIZE, &enc_frame_size);

    dec_state = speex_decoder_init(&speex_nb_mode);
    speex_encoder_ctl(dec_state, SPEEX_SET_QUALITY, &tmp);
    tmp = 8000;
    speex_encoder_ctl(dec_state, SPEEX_SET_SAMPLING_RATE, &tmp);
    tmp=1;
    speex_decoder_ctl(dec_state, SPEEX_SET_ENH, &tmp);
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
    static char receiveBuf[1024];
    static char decodeBuf[1280];
    static int decodeBufOffset = 0;

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
              }
           }
        }
        if(finishFlagstate) break;
        QThread::msleep(1);

    }
}
