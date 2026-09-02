#include "filescanner.h"

#include <QDir>

QStringList FileScanner::scan(
    const QString& inputDirectory,
    const QString& mask)
{
    const QDir directory(inputDirectory);

    if (!directory.exists())
    {
        return {};
    }

    QStringList filters;

    const QStringList parts =
        mask.split(',', Qt::SkipEmptyParts);

    for (QString filter : parts)
    {
        filter = filter.trimmed();

        if (filter.isEmpty())
        {
            continue;
        }

        if (filter.startsWith('.'))
        {
            filter.prepend('*');
        }
        else if (!filter.contains('*') &&
                 !filter.contains('?'))
        {
            // Explicit filename, e.g. testFile.bin.
            filters.append(filter);
            continue;
        }

        filters.append(filter);
    }

    if (filters.isEmpty())
    {
        return {};
    }

    return directory.entryList(
        filters,
        QDir::Files | QDir::Readable,
        QDir::Name
        );
}
