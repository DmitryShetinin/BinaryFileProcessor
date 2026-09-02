#include "outputnameresolver.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

QString OutputNameResolver::resolve(
    const QString& outputDirectory,
    const QString& fileName,
    ConflictPolicy policy)
{
    const QDir outputDir(outputDirectory);

    const QString originalPath =
        outputDir.absoluteFilePath(fileName);

    if (policy == ConflictPolicy::Overwrite)
    {
        return originalPath;
    }

    if (!QFile::exists(originalPath))
    {
        return originalPath;
    }

    const QFileInfo fileInfo(fileName);

    const QString baseName =
        fileInfo.completeBaseName();

    const QString suffix =
        fileInfo.suffix();

    for (int counter = 1; ; ++counter)
    {
        QString candidateName =
            baseName + "_" + QString::number(counter);

        if (!suffix.isEmpty())
        {
            candidateName += "." + suffix;
        }

        const QString candidatePath =
            outputDir.absoluteFilePath(candidateName);

        if (!QFile::exists(candidatePath))
        {
            return candidatePath;
        }
    }
}
