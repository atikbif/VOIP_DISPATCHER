#include "udpcontroller.h"

UDPController::UDPController(const QString &ip, QObject *parent) : QObject(parent)
{
    worker = new UDPWorker(ip);
    worker->moveToThread(&udpThread);
    connect(&udpThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &UDPController::init, worker, &UDPWorker::scan);
    connect(worker, &UDPWorker::linkStateChanged,this,&UDPController::linkStateChanged);
    connect(worker, &UDPWorker::updateAudio,this,&UDPController::updateAudio);
    connect(worker, &UDPWorker::fromIDSignal, this, &UDPController::fromIDSignal);
    connect(worker, &UDPWorker::updateState, this, &UDPController::updateState);
    connect(worker, &UDPWorker::updateGroupState, this, &UDPController::updateGroupState);
    connect(worker, &UDPWorker::startRecord, this, &UDPController::startRecord);
    connect(worker, &UDPWorker::stopRecord, this, &UDPController::stopRecord);
    udpThread.start();
    emit init();
}

UDPController::~UDPController()
{
    worker->finish();
    udpThread.quit();
    udpThread.wait();
}

void UDPController::start()
{
    worker->start();
}

void UDPController::stop()
{
    worker->stop();
}

void UDPController::writeAudioPacket(const QByteArray &input)
{
  worker->writeAudioPacket(input);
}

void UDPController::setToID(unsigned char group, unsigned char point)
{
    worker->setToID(group, point);
}

void UDPController::checkAudio()
{
    worker->checkAudio();
}

void UDPController::setVolume(int group, int point, int value, bool allPoints)
{
    worker->setVolume(group, point, value, allPoints);
}

void UDPController::setInpConf(int group, int point, int filter, int enValue)
{
    worker->setInpConf(group,point,filter,enValue);
}
