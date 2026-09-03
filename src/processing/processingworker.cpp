// processingworker.cpp

#include "processingworker.h"

#include "fileprocessor.h"
#include "outputnameresolver.h"
#include "filescanner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
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

    /*
     * Validate configuration.
     */
    if (options.inputPath.trimmed().isEmpty())
    {
        emit error("Input directory is not specified.");
        emit finished();
        return;
    }

    if (options.outputPath.trimmed().isEmpty())
    {
        emit error("Output directory is not specified.");
        emit finished();
        return;
    }

    if (options.fileMask.trimmed().isEmpty())
    {
        emit error("File mask is not specified.");
        emit finished();
        return;
    }

    if (options.pollingIntervalSeconds <= 0)
    {
        emit error("Polling interval must be greater than zero.");
        emit finished();
        return;
    }

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

    /*
     * Do not allow input and output directories to be the same.
     *
     * Otherwise overwrite mode could truncate the input file
     * before it is fully processed. In timer mode it could also
     * cause generated output files to be processed again.
     */
    const QString inputAbsolutePath =
        QDir::cleanPath(
            inputDirectory.absolutePath()
            );

    QDir outputDirectory(options.outputPath);

    if (!outputDirectory.exists())
    {
        if (!QDir().mkpath(options.outputPath))
        {
            emit error(
                "Failed to create output directory: " +
                options.outputPath
                );

            emit finished();
            return;
        }

        outputDirectory.setPath(options.outputPath);
    }

    const QString outputAbsolutePath =
        QDir::cleanPath(
            outputDirectory.absolutePath()
            );

    if (inputAbsolutePath.compare(
            outputAbsolutePath,
            Qt::CaseInsensitive
            ) == 0)
    {
        emit error(
            "Input and output directories must be different."
            );

        emit finished();
        return;
    }

    emit statusChanged("Searching for files...");

    FileProcessor processor;

    bool stopped = false;

    QSet<QString> processedFiles;

    while (true)
    {
        if (processState() == FileProcessor::ProcessState::Stop)
        {
            stopped = true;
            break;
        }

        emit statusChanged("Searching for files...");

        const QStringList files =
            FileScanner::scan(
                options.inputPath,
                options.fileMask
                );

        if (files.isEmpty())
        {
            if (!options.timerMode)
            {
                emit statusChanged("No files found.");
                break;
            }

            emit statusChanged(
                "No new files found. Waiting for next scan..."
                );
        }

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

            if (!fileInfo.exists())
            {
                emit error(
                    "Input file disappeared before processing: " +
                    inputFile
                    );

                continue;
            }

            if (!fileInfo.isFile())
            {
                continue;
            }

            if (!fileInfo.isReadable())
            {
                emit error(
                    "Input file is not readable: " +
                    inputFile
                    );

                continue;
            }

            if (processedFiles.contains(inputFile))
            {
                continue;
            }

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

                [this](
                    qint64 processedBytes,
                    qint64 totalBytes
                    )
                {
                    if (totalBytes > 0)
                    {
                        const int percent =
                            static_cast<int>(
                                (processedBytes * 100) /
                                totalBytes
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

                emit error(
                    "Failed to process file '" +
                    inputFile +
                    "': " +
                    errorMessage
                    );

                /*
                 * Do not mark the file as processed when
                 * processing failed.
                 */
                if (!options.timerMode)
                {
                    stopped = true;
                    break;
                }

                continue;
            }

            processedFiles.insert(inputFile);

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
            break;
        }

        if (!options.timerMode)
        {
            break;
        }

        emit statusChanged(
            "Waiting for new files... Next scan in " +
            QString::number(options.pollingIntervalSeconds) +
            " sec."
            );

        QMutexLocker locker(&stateMutex);

        if (stopRequested)
        {
            stopped = true;
            break;
        }

        pauseCondition.wait(
            &stateMutex,
            options.pollingIntervalSeconds * 1000
            );

        if (stopRequested)
        {
            stopped = true;
            break;
        }
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
