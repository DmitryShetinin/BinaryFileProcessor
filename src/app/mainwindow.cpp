#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , controller(new ProcessingController(this))
{
    ui->setupUi(this);

    connect(
        controller,
        &ProcessingController::progressChanged,
        this,
        &MainWindow::onProgressChanged
        );

    connect(
        controller,
        &ProcessingController::statusChanged,
        this,
        &MainWindow::onProcessingStatusChanged
        );

    connect(
        controller,
        &ProcessingController::fileStarted,
        this,
        &MainWindow::onFileStarted
        );

    connect(
        controller,
        &ProcessingController::fileFinished,
        this,
        &MainWindow::onFileFinished
        );

    connect(
        controller,
        &ProcessingController::error,
        this,
        &MainWindow::onProcessingError
        );

    connect(
        controller,
        &ProcessingController::finished,
        this,
        &MainWindow::onProcessingFinished
        );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnAddFiles_clicked()
{
    // Реализуем отдельно.
}

void MainWindow::on_btnStart_clicked()
{
    const ProcessingOptions options =
        collectProcessingOptions();

    controller->start(options);

    ui->btnStart->setEnabled(false);
    ui->btnPause->setEnabled(true);
    ui->btnResume->setEnabled(false);
    ui->btnStop->setEnabled(true);

    ui->progressBar->setValue(0);
}

void MainWindow::on_btnPause_clicked()
{
    controller->pause();

    ui->btnPause->setEnabled(false);
    ui->btnResume->setEnabled(true);
}

void MainWindow::on_btnResume_clicked()
{
    controller->resume();

    ui->btnPause->setEnabled(true);
    ui->btnResume->setEnabled(false);
}

void MainWindow::on_btnStop_clicked()
{
    controller->stop();

    ui->btnPause->setEnabled(false);
    ui->btnResume->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->btnStart->setEnabled(true);
}

ProcessingOptions MainWindow::collectProcessingOptions() const
{
    ProcessingOptions options;

    options.inputPath =
        ui->editInputPath->text().trimmed();

    options.outputPath =
        ui->editOutputPath->text().trimmed();

    options.fileMask =
        ui->editMask->text().trimmed();

    options.deleteInputFiles =
        ui->checkDeleteInput->isChecked();

    options.overwriteExisting =
        ui->comboConflict->currentIndex() == 0;

    options.timerMode =
        ui->checkTimerMode->isChecked();

    options.pollingIntervalSeconds =
        ui->spinPollInterval->value();

    options.xorKey =
        ui->editHexKey->text().toULongLong(
            nullptr,
            16
            );

    return options;
}

void MainWindow::onProcessingFinished()
{
    ui->btnStart->setEnabled(true);
    ui->btnPause->setEnabled(false);
    ui->btnResume->setEnabled(false);
    ui->btnStop->setEnabled(false);
}

void MainWindow::onProcessingError(const QString& message)
{
    ui->logOutput->appendPlainText(
        "[ERROR] " + message
        );
}

void MainWindow::onProcessingStatusChanged(
    const QString& status)
{
    ui->labelStatus->setText(status);

    ui->logOutput->appendPlainText(
        status
        );
}

void MainWindow::onFileStarted(const QString& file)
{
    ui->logOutput->appendPlainText(
        "[START] " + file
        );
}

void MainWindow::onFileFinished(const QString& file)
{
    ui->logOutput->appendPlainText(
        "[DONE] " + file
        );
}

void MainWindow::onProgressChanged(int percent)
{
    ui->progressBar->setValue(percent);
}


void MainWindow::on_btnBrowseInput_clicked()
{
    const QString directory = QFileDialog::getExistingDirectory(
        this,
        "Выберите папку с входными файлами",
        ui->editInputPath->text()
        );

    if (!directory.isEmpty())
    {
        ui->editInputPath->setText(directory);
    }
}

void MainWindow::on_btnBrowseOutput_clicked()
{
    const QString directory = QFileDialog::getExistingDirectory(
        this,
        "Выберите папку для результатов",
        ui->editOutputPath->text()
        );

    if (!directory.isEmpty())
    {
        ui->editOutputPath->setText(directory);
    }
}


