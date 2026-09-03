#include <QtTest>

#include "filescanner.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class FileScannerTest : public QObject
{
    Q_OBJECT

private slots:
    void findByExtension();
    void findByWildcard();
    void findByMultipleMasks();
    void findByExactFileName();
    void ignoreEmptyMasks();
};

void FileScannerTest::findByExtension()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile txtFile(tempDir.filePath("first.txt"));
    QVERIFY(txtFile.open(QIODevice::WriteOnly));
    txtFile.close();

    QFile binFile(tempDir.filePath("second.bin"));
    QVERIFY(binFile.open(QIODevice::WriteOnly));
    binFile.close();

    const QStringList result =
        FileScanner::scan(tempDir.path(), ".txt");

    QCOMPARE(result.size(), 1);
    QCOMPARE(result.first(), "first.txt");
}

void FileScannerTest::findByWildcard()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    for (const QString& fileName : {
             QStringLiteral("a.bin"),
             QStringLiteral("b.bin"),
             QStringLiteral("c.txt")
         })
    {
        QFile file(tempDir.filePath(fileName));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    const QStringList result =
        FileScanner::scan(tempDir.path(), "*.bin");

    QCOMPARE(result.size(), 2);
    QVERIFY(result.contains("a.bin"));
    QVERIFY(result.contains("b.bin"));
    QVERIFY(!result.contains("c.txt"));
}

void FileScannerTest::findByMultipleMasks()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    for (const QString& fileName : {
             QStringLiteral("a.txt"),
             QStringLiteral("b.bin"),
             QStringLiteral("c.txt"),
             QStringLiteral("d.jpg")
         })
    {
        QFile file(tempDir.filePath(fileName));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }

    const QStringList result =
        FileScanner::scan(
            tempDir.path(),
            ".txt, .bin"
        );

    QCOMPARE(result.size(), 3);

    QVERIFY(result.contains("a.txt"));
    QVERIFY(result.contains("b.bin"));
    QVERIFY(result.contains("c.txt"));
    QVERIFY(!result.contains("d.jpg"));
}

void FileScannerTest::findByExactFileName()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile target(tempDir.filePath("testFile.bin"));
    QVERIFY(target.open(QIODevice::WriteOnly));
    target.close();

    QFile other(tempDir.filePath("other.bin"));
    QVERIFY(other.open(QIODevice::WriteOnly));
    other.close();

    const QStringList result =
        FileScanner::scan(
            tempDir.path(),
            "testFile.bin"
        );

    QCOMPARE(result.size(), 1);
    QCOMPARE(result.first(), "testFile.bin");
}

void FileScannerTest::ignoreEmptyMasks()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile file(tempDir.filePath("test.txt"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    const QStringList result =
        FileScanner::scan(
            tempDir.path(),
            " , , "
        );

    QVERIFY(result.isEmpty());
}

QTEST_MAIN(FileScannerTest)

#include "filescanner_test.moc"