#pragma once

#include <QString>

enum class ConflictPolicy
{
    Overwrite,
    AddCounter
};

class OutputNameResolver
{
public:
    static QString resolve(
        const QString& outputDirectory,
        const QString& fileName,
        ConflictPolicy policy);
};

