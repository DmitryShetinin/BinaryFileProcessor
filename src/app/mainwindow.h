#pragma once

#include "processingcontroller.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAddFiles_clicked();
    void on_btnStart_clicked();
    void on_btnPause_clicked();
    void on_btnResume_clicked();
    void on_btnStop_clicked();

    void on_btnBrowseInput_clicked();
    void on_btnBrowseOutput_clicked();

    void onProcessingFinished();
    void onProcessingError(const QString& message);
    void onProcessingStatusChanged(const QString& status);
    void onFileStarted(const QString& file);
    void onFileFinished(const QString& file);
    void onProgressChanged(int percent);



private:
    ProcessingOptions collectProcessingOptions() const;

private:
    Ui::MainWindow *ui;
    ProcessingController* controller;
};
