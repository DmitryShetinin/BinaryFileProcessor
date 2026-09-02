#include "fileprocessor.h"

#include <QFile>
#include <QByteArray>
#include <stdexcept>

#include "fileprocessor.h"

#include <QByteArray>
#include <QFile>

#include <stdexcept>

bool FileProcessor::process(
    const QString& inputFile,
    const QString& outputFile,
    quint64 xorKey,
    const ProgressCallback& onProgress,
    QString& errorMessage)
{
    QFile input(inputFile);
    QFile output(outputFile);

    if (!input.open(QIODevice::ReadOnly))
    {
        errorMessage =
            "Failed to open input file: " +
            input.errorString();

        return false;
    }

    if (!output.open(QIODevice::WriteOnly))
    {
        errorMessage =
            "Failed to open output file: " +
            output.errorString();

        return false;
    }

    constexpr qsizetype ChunkSize = 4 * 1024 * 1024;

    QByteArray buffer;
    buffer.resize(ChunkSize);

    const qint64 totalBytes = input.size();

    qint64 processedBytes = 0;

    while (!input.atEnd())
    {
        const qint64 bytesRead =
            input.read(buffer.data(), ChunkSize);

        if (bytesRead < 0)
        {
            errorMessage =
                "Failed to read input file: " +
                input.errorString();

            return false;
        }

        for (qint64 i = 0; i < bytesRead; ++i)
        {
            const int keyOffset =
                static_cast<int>((processedBytes + i) % 8);

            const quint8 keyByte =
                static_cast<quint8>(
                    (xorKey >> ((7 - keyOffset) * 8)) & 0xFF
                    );

            buffer[i] =
                static_cast<char>(
                    static_cast<quint8>(buffer[i]) ^ keyByte
                    );
        }

        const qint64 bytesWritten =
            output.write(buffer.constData(), bytesRead);

        if (bytesWritten != bytesRead)
        {
            errorMessage =
                "Failed to write output file: " +
                output.errorString();

            return false;
        }

        processedBytes += bytesRead;

        if (onProgress)
        {
            const ProcessState state =
                onProgress(processedBytes, totalBytes);

            if (state == ProcessState::Stop)
            {
                return false;
            }
        }
    }

    return true;
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

