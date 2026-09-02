// fileprocessor.h

#pragma once

#include <QString>
#include <QtGlobal>

#include <functional>

class FileProcessor
{
public:
    enum class ProcessState
    {
        Continue,
        Stop
    };

    using ProgressCallback =
        std::function<ProcessState(qint64 processedBytes, qint64 totalBytes)>;

    bool process(
        const QString& inputFile,
        const QString& outputFile,
        quint64 xorKey,
        const ProgressCallback& onProgress,
        QString& errorMessage
        );
};
