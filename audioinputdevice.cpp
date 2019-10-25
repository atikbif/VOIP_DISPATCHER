#include "audioinputdevice.h"
#include <QDebug>
#include "checksum.h"
#include <QtConcurrent/QtConcurrent>

#include "wav_example.h"

quint16 AudioInputDevice::id=0;

void AudioInputDevice::sendUDPData(const QByteArray &input)
{
    scanner->writeAudioPacket(input);
}

AudioInputDevice::AudioInputDevice(const QAudioFormat &format,UDPController *scanner):scanner(scanner)
{
    Q_UNUSED(format)
    switch (m_format.sampleType())
    {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 32767;
            break;
        default:
            break;
    }
    int error;
    //enc = opus_encoder_create(8000, 1, OPUS_APPLICATION_VOIP, &error);
    enc = opus_encoder_create(8000, 1, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &error);
    //enc = opus_encoder_create(8000, 1, OPUS_APPLICATION_AUDIO, &error);
    //opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_NARROWBAND));
    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(4));
    //opus_encoder_ctl(enc, OPUS_SET_BITRATE(16000));
    dec = opus_decoder_create(8000, 1, &error);

    /*opus_int16 test[160];
    for(int i=0;i<160;i++) test[i]=0;
    unsigned char res[100];
    auto nbBytes = opus_encode(enc, test, 160,res, 100);
    QString empty;
    for(int i=0;i<nbBytes;i++) {
        empty+=QString::number(res[i],16)+" ";
    }
    qDebug()<< nbBytes << empty;*/

}

void AudioInputDevice::start()
{
  open(QIODevice::WriteOnly);
}

void AudioInputDevice::stop()
{
  close();
}

qint64 AudioInputDevice::readData(char *data, qint64 maxlen)
{
  Q_UNUSED(data)
  Q_UNUSED(maxlen)
  return 0;
}

qint64 AudioInputDevice::writeData(const char *data, qint64 len)
{
    //static long ex_offset = 0;
    static QByteArray curBuf;
    QByteArray udpBuf;
    //static const int udpBufMaxLength = 40;
    qint64 res = len;


    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
    QVector<double> plot;
    for (int i = 0; i < len/2; ++i) {
        qint16 value = qFromLittleEndian<qint16>(ptr);
        ptr += 2;
        plot.append(static_cast<double>(value)/32767);
    }
    emit newLevel(plot);
    for (int i = 0; i < len; i++) {
      curBuf.append(data[i]);
    }

    //qDebug()<<curBuf;

    int offset = 0;
    int pckt_cnt = static_cast<int>(len/320);
    int pckt_num = 0;
    udpBuf.append(static_cast<char>(pckt_cnt));
    for(int i=0;i<pckt_cnt;i++) udpBuf.append('\0');
    while (len>=320) {
        int nbBytes = opus_encode(enc, (opus_int16*)&data[offset], 160,(unsigned char*) cbits, 1024);
        //nbBytes = opus_encode(enc, (opus_int16*)&wav_ex[ex_offset], 160,(unsigned char*) cbits, 1024);
        //ex_offset+=320;if(ex_offset>=sizeof (wav_ex)) ex_offset=0;
        udpBuf[1+pckt_num] = static_cast<char>(nbBytes);
        pckt_num++;
        //qDebug() << nbBytes;
        curBuf.clear();
        //if(nbBytes>udpBufMaxLength) nbBytes=udpBufMaxLength;
        for(int j=0;j<nbBytes;j++) {
          udpBuf.append(cbits[j]);
        }
        len-=320;offset+=320;
    }
    //qDebug()<<pckt_cnt;

    sendUDPData(udpBuf);
    return res;
}
