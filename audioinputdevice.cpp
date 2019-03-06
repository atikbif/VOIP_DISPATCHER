#include "audioinputdevice.h"
#include <QDebug>
#include "checksum.h"
#include <QtConcurrent/QtConcurrent>

quint16 AudioInputDevice::id=0;

void AudioInputDevice::sendUDPData(const QByteArray &input)
{
    scanner->writeAudioPacket(input);
}

AudioInputDevice::AudioInputDevice(const QAudioFormat &format,UDPController *scanner):scanner(scanner)
{
    int tmp = 0;
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
    enc = opus_encoder_create(8000, 1, OPUS_APPLICATION_AUDIO, &error);
    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(4));
    dec = opus_decoder_create(8000, 1, &error);
    //opus_decoder_ctl(dec,OPUS_SET_COMPLEXITY(4));
    /*state = speex_encoder_init(&speex_nb_mode);
    speex_bits_init(&bits);
    tmp=4;
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
    speex_decoder_ctl(dec_state, SPEEX_SET_ENH, &tmp);*/

    /*udp.connectToHost(QHostAddress("192.168.5.85"),7);
    if(!udp.open(QIODevice::ReadWrite)) {
        qDebug() << "problem";
    }else {
        qDebug() << "open";
    }*/
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
    static QByteArray curBuf;
    QByteArray udpBuf;
    int nbBytes;
    static const int udpBufMaxLength = 40;
    qint64 res = len;

    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
    QVector<double> plot;
    for (int i = 0; i < len/2; ++i) {
        qint16 value = qFromLittleEndian<qint16>(ptr);
        ptr += 2;
        plot.append((double)value/32767);
    }
    emit newLevel(plot);
    for (int i = 0; i < len; i++) {
      curBuf.append(data[i]);
    }
    nbBytes = opus_encode(enc, (opus_int16*)data, len/2,(unsigned char*) cbits, 1024);
    qDebug() << nbBytes;

    /*for(int i=0;i<len;i++) {
      str+=QString::number(res[i],16) + " ";
      if(i && (i%16==0 || i==len-1)) {qDebug() << "ENC:" + str;str="";}
    }*/
    /*speex_bits_reset(&bits);
    speex_encode_int(state, (spx_int16_t*)ptr, &bits);
    nbBytes = speex_bits_write(&bits, cbits, 1024);*/
    curBuf.clear();
    if(nbBytes>udpBufMaxLength) nbBytes=udpBufMaxLength;
    for(int j=0;j<nbBytes;j++) {
      udpBuf.append(cbits[j]);
    }
    //qDebug()<<udpBuf.size();
    sendUDPData(udpBuf);
    return res;
}
