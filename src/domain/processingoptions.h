// processingoptions.h

#pragma once

#include <QString>

struct ProcessingOptions
{
    QString inputPath;
    QString outputPath;
    QString fileMask;

    bool deleteInputFiles = false;
    bool overwriteExisting = false;
    bool timerMode = false;

    int pollingIntervalSeconds = 5;

    quint64 xorKey = 0;
};
