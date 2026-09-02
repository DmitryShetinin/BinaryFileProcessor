#pragma once

#include <QStringList>

class FileScanner
{
public:
    static QStringList scan(
        const QString& inputDirectory,
        const QString& mask);
};
