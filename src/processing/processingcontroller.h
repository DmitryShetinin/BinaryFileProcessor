// processingcontroller.h


#pragma once

#include "processingoptions.h"

#include <QObject>

class QThread;
class ProcessingWorker;

class ProcessingController : public QObject
{
    Q_OBJECT

public:
    explicit ProcessingController(QObject* parent = nullptr);
    ~ProcessingController();

    void start(const ProcessingOptions& options);

    void pause();
    void resume();
    void stop();

signals:
    void progressChanged(int percent);
    void statusChanged(const QString& status);
    void fileStarted(const QString& file);
    void fileFinished(const QString& file);
    void error(const QString& message);
    void finished();

private:
    QThread* thread = nullptr;
    ProcessingWorker* worker = nullptr;
};
