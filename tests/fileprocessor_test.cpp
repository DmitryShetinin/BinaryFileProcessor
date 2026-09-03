#include <QtTest>

#include "fileprocessor.h"

#include <QFile>
#include <QTemporaryDir>

class FileProcessorTest : public QObject
{
    Q_OBJECT

private slots:
    void processSmallFile();
    void processEmptyFile();
    void processFileLargerThanChunk();
};

void FileProcessorTest::processSmallFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString inputPath =
        tempDir.filePath("input.bin");

    const QString outputPath =
        tempDir.filePath("output.bin");

    const QByteArray inputData =
        QByteArray::fromHex("48656C6C6F20576F726C6421");

    {
        QFile inputFile(inputPath);

        QVERIFY(inputFile.open(QIODevice::WriteOnly));
        QCOMPARE(inputFile.write(inputData), inputData.size());
        inputFile.close();
    }

    const quint64 key =
        0x1234567890ABCDEFULL;

    FileProcessor processor;

    QString errorMessage;

    const bool success =
        processor.process(
            inputPath,
            outputPath,
            key,
            [](qint64, qint64)
            {
                return FileProcessor::ProcessState::Continue;
            },
            errorMessage
        );

    QVERIFY2(
        success,
        qPrintable(errorMessage)
    );

    QFile outputFile(outputPath);

    QVERIFY(outputFile.open(QIODevice::ReadOnly));

    const QByteArray outputData =
        outputFile.readAll();

    outputFile.close();

    const QByteArray keyBytes =
        QByteArray::fromHex("1234567890ABCDEF");

    QByteArray expected;
    expected.resize(inputData.size());

    for (qsizetype i = 0; i < inputData.size(); ++i)
    {
        expected[i] =
            static_cast<char>(
                static_cast<quint8>(inputData[i]) ^
                static_cast<quint8>(
                    keyBytes[i % 8]
                )
            );
    }

    QCOMPARE(outputData, expected);
}

void FileProcessorTest::processEmptyFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString inputPath =
        tempDir.filePath("empty.bin");

    const QString outputPath =
        tempDir.filePath("empty.out");

    {
        QFile inputFile(inputPath);
        QVERIFY(inputFile.open(QIODevice::WriteOnly));
        inputFile.close();
    }

    const quint64 key =
        0x1234567890ABCDEFULL;

    FileProcessor processor;

    QString errorMessage;

    const bool success =
        processor.process(
            inputPath,
            outputPath,
            key,
            [](qint64, qint64)
            {
                return FileProcessor::ProcessState::Continue;
            },
            errorMessage
        );

    QVERIFY2(
        success,
        qPrintable(errorMessage)
    );

    QFile outputFile(outputPath);

    QVERIFY(outputFile.exists());
    QVERIFY(outputFile.open(QIODevice::ReadOnly));

    const QByteArray outputData =
        outputFile.readAll();

    outputFile.close();

    QVERIFY(outputData.isEmpty());
}

void FileProcessorTest::processFileLargerThanChunk()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString inputPath =
        tempDir.filePath("large.bin");

    const QString outputPath =
        tempDir.filePath("large.out");

    constexpr qint64 chunkSize =
        4 * 1024 * 1024;

    const qint64 dataSize =
        chunkSize + 123;

    QByteArray inputData;
    inputData.resize(dataSize);

    for (qint64 i = 0; i < dataSize; ++i)
    {
        inputData[static_cast<qsizetype>(i)] =
            static_cast<char>(i % 251);
    }

    {
        QFile inputFile(inputPath);

        QVERIFY(inputFile.open(QIODevice::WriteOnly));
        QCOMPARE(
            inputFile.write(inputData),
            inputData.size()
        );

        inputFile.close();
    }

    const quint64 key =
        0x1234567890ABCDEFULL;

    FileProcessor processor;

    QString errorMessage;

    const bool success =
        processor.process(
            inputPath,
            outputPath,
            key,
            [](qint64, qint64)
            {
                return FileProcessor::ProcessState::Continue;
            },
            errorMessage
        );

    QVERIFY2(
        success,
        qPrintable(errorMessage)
    );

    QFile outputFile(outputPath);

    QVERIFY(outputFile.open(QIODevice::ReadOnly));

    const QByteArray outputData =
        outputFile.readAll();

    outputFile.close();

    QCOMPARE(outputData.size(), inputData.size());

    const QByteArray keyBytes =
        QByteArray::fromHex("1234567890ABCDEF");

    for (qsizetype i = 0; i < inputData.size(); ++i)
    {
        const char expected =
            static_cast<char>(
                static_cast<quint8>(inputData[i]) ^
                static_cast<quint8>(
                    keyBytes[i % 8]
                )
            );

        QCOMPARE(
            outputData[i],
            expected
        );
    }
}

QTEST_MAIN(FileProcessorTest)

#include "fileprocessor_test.moc"