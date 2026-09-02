// processingworker.h

#pragma once

#include "processingoptions.h"
#include "fileprocessor.h"

#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class ProcessingWorker : public QObject
{
    Q_OBJECT

public:
    explicit ProcessingWorker(QObject* parent = nullptr);

public slots:
    void process(const ProcessingOptions& options);

signals:
    void progressChanged(int percent);
    void statusChanged(const QString& status);
    void fileStarted(const QString& file);
    void fileFinished(const QString& file);

    void error(const QString& message);
    void finished();

public:
    void requestPause();
    void resume();
    void requestStop();

private:
    FileProcessor::ProcessState processState();

private:
    QMutex stateMutex;
    QWaitCondition pauseCondition;

    bool paused = false;
    bool stopRequested = false;
};
