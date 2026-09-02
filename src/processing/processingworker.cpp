// processingworker.cpp

#include "processingworker.h"

#include "fileprocessor.h"
#include "outputnameresolver.h"
#include "filescanner.h"


#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

ProcessingWorker::ProcessingWorker(QObject* parent)
    : QObject(parent)
{
}

void ProcessingWorker::process(const ProcessingOptions& options)
{
    {
        QMutexLocker locker(&stateMutex);

        paused = false;
        stopRequested = false;
    }

    emit statusChanged("Searching for files...");

    QDir inputDirectory(options.inputPath);

    if (!inputDirectory.exists())
    {
        emit error(
            "Input directory does not exist: " +
            options.inputPath
            );

        emit finished();
        return;
    }

    const QStringList files =
        FileScanner::scan(
            options.inputPath,
            options.fileMask
            );

    if (files.isEmpty())
    {
        emit statusChanged("No files found.");
        emit finished();
        return;
    }

    FileProcessor processor;
    bool stopped = false;

    for (const QString& fileName : files)
    {
        if (processState() == FileProcessor::ProcessState::Stop)
        {
            stopped = true;
            break;
        }

        const QString inputFile =
            inputDirectory.absoluteFilePath(fileName);

        const QFileInfo fileInfo(inputFile);

        const ConflictPolicy conflictPolicy =
            options.overwriteExisting
                ? ConflictPolicy::Overwrite
                : ConflictPolicy::AddCounter;

        const QString outputFile =
            OutputNameResolver::resolve(
                options.outputPath,
                fileInfo.fileName(),
                conflictPolicy
                );

        emit fileStarted(inputFile);

        emit statusChanged(
            "Processing: " + fileInfo.fileName()
            );

        QString errorMessage;

        const bool success = processor.process(
            inputFile,
            outputFile,
            options.xorKey,

            [this](qint64 processedBytes, qint64 totalBytes)
            {
                if (totalBytes > 0)
                {
                    const int percent =
                        static_cast<int>(
                            (processedBytes * 100) / totalBytes
                            );

                    emit progressChanged(percent);
                }

                return processState();
            },

            errorMessage
            );

        if (!success)
        {
            bool wasStopped = false;

            {
                QMutexLocker locker(&stateMutex);
                wasStopped = stopRequested;
            }

            if (wasStopped)
            {
                stopped = true;
                break;
            }

            emit error(errorMessage);
            break;
        }

        if (options.deleteInputFiles)
        {
            if (!QFile::remove(inputFile))
            {
                emit error(
                    "Failed to delete input file: " +
                    inputFile
                    );
            }
        }

        emit fileFinished(inputFile);
    }

    if (stopped)
    {
        emit statusChanged("Processing stopped.");
    }
    else
    {
        emit statusChanged("Processing finished.");
    }

    emit finished();
}

void ProcessingWorker::requestPause()
{
    QMutexLocker locker(&stateMutex);

    paused = true;

    emit statusChanged("Paused.");
}

void ProcessingWorker::resume()
{
    QMutexLocker locker(&stateMutex);

    paused = false;

    pauseCondition.wakeAll();
}

void ProcessingWorker::requestStop()
{
    QMutexLocker locker(&stateMutex);

    stopRequested = true;
    paused = false;

    pauseCondition.wakeAll();
}

FileProcessor::ProcessState ProcessingWorker::processState()
{
    QMutexLocker locker(&stateMutex);

    while (paused && !stopRequested)
    {
        pauseCondition.wait(&stateMutex);
    }

    if (stopRequested)
    {
        return FileProcessor::ProcessState::Stop;
    }

    return FileProcessor::ProcessState::Continue;
}
