#include "udpcontroller.h"

UDPController::UDPController(QObject *parent) : QObject(parent)
{
    worker = new UDPWorker;
    worker->moveToThread(&udpThread);
    connect(&udpThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &UDPController::init, worker, &UDPWorker::scan);
    connect(worker, &UDPWorker::linkStateChanged,this,&UDPController::linkStateChanged);
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
