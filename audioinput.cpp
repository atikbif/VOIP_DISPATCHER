#include "audioinput.h"
#include <QDebug>

AudioInput::AudioInput(const QAudioFormat &format)
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
    state = speex_encoder_init(&speex_nb_mode);
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
    speex_decoder_ctl(dec_state, SPEEX_SET_ENH, &tmp);
}

void AudioInput::start()
{
  open(QIODevice::WriteOnly);
}

void AudioInput::stop()
{
  close();
}

qint64 AudioInput::readData(char *data, qint64 maxlen)
{
  Q_UNUSED(data)
  Q_UNUSED(maxlen)
  return 0;
}

qint64 AudioInput::writeData(const char *data, qint64 len)
{
    static QByteArray curBuf;
    static QByteArray udpBuf;
    int nbBytes;
    static const int udpBufMaxLength = 40;
    qint64 res = len;
    qDebug()<< "Length:" << len;
    for (int i = 0; i < len; i++) {
      curBuf.append(data[i]);
      if(curBuf.count()==FRAME_SIZE*2) {
        char *ptr = curBuf.data();
        speex_bits_reset(&bits);
        speex_encode_int(state, (spx_int16_t*)ptr, &bits);
        nbBytes = speex_bits_write(&bits, cbits, 1024);
        curBuf.clear();
        if(nbBytes>udpBufMaxLength) nbBytes=udpBufMaxLength;
        for(int j=0;j<nbBytes;j++) {
          udpBuf.append(cbits[j]);
        }
      }
    }
    qDebug()<<udpBuf.size();
    udpBuf.clear();
    return res;
}
