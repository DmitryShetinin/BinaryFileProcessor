#include <QtTest>

#include "outputnameresolver.h"

#include <QFile>
#include <QTemporaryDir>

class OutputNameResolverTest : public QObject
{
    Q_OBJECT

private slots:
    void returnOriginalNameWhenFree();
    void overwriteExistingFile();
    void addCounterForExistingFile();
    void skipExistingCounters();
    void preserveExtension();
};

void OutputNameResolverTest::returnOriginalNameWhenFree()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString result =
        OutputNameResolver::resolve(
            tempDir.path(),
            "test.bin",
            ConflictPolicy::AddCounter
        );

    QCOMPARE(
        result,
        tempDir.filePath("test.bin")
    );
}

void OutputNameResolverTest::overwriteExistingFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile file(tempDir.filePath("test.bin"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    const QString result =
        OutputNameResolver::resolve(
            tempDir.path(),
            "test.bin",
            ConflictPolicy::Overwrite
        );

    QCOMPARE(
        result,
        tempDir.filePath("test.bin")
    );
}

void OutputNameResolverTest::addCounterForExistingFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile file(tempDir.filePath("test.bin"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    const QString result =
        OutputNameResolver::resolve(
            tempDir.path(),
            "test.bin",
            ConflictPolicy::AddCounter
        );

    QCOMPARE(
        result,
        tempDir.filePath("test_1.bin")
    );
}

void OutputNameResolverTest::skipExistingCounters()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    for (const QString& fileName : {
             QStringLiteral("test.bin"),
             QStringLiteral("test_1.bin"),
             QStringLiteral("test_2.bin")
         })
    {
        QFile file(tempDir.filePath(fileName));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    const QString result =
        OutputNameResolver::resolve(
            tempDir.path(),
            "test.bin",
            ConflictPolicy::AddCounter
        );

    QCOMPARE(
        result,
        tempDir.filePath("test_3.bin")
    );
}

void OutputNameResolverTest::preserveExtension()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile file(tempDir.filePath("archive.tar.gz"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    const QString result =
        OutputNameResolver::resolve(
            tempDir.path(),
            "archive.tar.gz",
            ConflictPolicy::AddCounter
        );

    QCOMPARE(
        result,
        tempDir.filePath("archive.tar_1.gz")
    );
}

QTEST_MAIN(OutputNameResolverTest)

#include "outputnameresolver_test.moc"