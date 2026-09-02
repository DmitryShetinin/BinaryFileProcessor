// processingcontroller.cpp


#include "processingcontroller.h"

#include "processingworker.h"

#include <QThread>

ProcessingController::ProcessingController(QObject* parent)
    : QObject(parent)
{
}

ProcessingController::~ProcessingController()
{
    stop();
}

void ProcessingController::start(const ProcessingOptions& options)
{
    if (thread != nullptr)
    {
        return;
    }

    thread = new QThread;
    worker = new ProcessingWorker;

    worker->moveToThread(thread);

    connect(
        thread,
        &QThread::started,
        worker,
        [this, options]()
        {
            worker->process(options);
        }
        );

    connect(
        worker,
        &ProcessingWorker::progressChanged,
        this,
        &ProcessingController::progressChanged
        );

    connect(
        worker,
        &ProcessingWorker::statusChanged,
        this,
        &ProcessingController::statusChanged
        );

    connect(
        worker,
        &ProcessingWorker::fileStarted,
        this,
        &ProcessingController::fileStarted
        );

    connect(
        worker,
        &ProcessingWorker::fileFinished,
        this,
        &ProcessingController::fileFinished
        );

    connect(
        worker,
        &ProcessingWorker::error,
        this,
        &ProcessingController::error
        );

    connect(
        worker,
        &ProcessingWorker::finished,
        this,
        &ProcessingController::finished
        );

    connect(
        worker,
        &ProcessingWorker::finished,
        thread,
        &QThread::quit
        );

    connect(
        worker,
        &ProcessingWorker::finished,
        worker,
        &ProcessingWorker::deleteLater
        );

    connect(
        thread,
        &QThread::finished,
        thread,
        &QThread::deleteLater
        );

    connect(
        thread,
        &QThread::finished,
        this,
        [this]()
        {
            thread = nullptr;
            worker = nullptr;
        }
        );

    thread->start();
}

void ProcessingController::pause()
{
    if (worker != nullptr)
    {
        worker->requestPause();
    }
}

void ProcessingController::resume()
{
    if (worker != nullptr)
    {
        worker->resume();
    }
}

void ProcessingController::stop()
{
    if (worker == nullptr || thread == nullptr)
    {
        return;
    }

    worker->requestStop();

    thread->wait();
}
