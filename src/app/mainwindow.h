#pragma once

#include <QMainWindow>

#include "processingcontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAddFiles_clicked();
    void on_btnStart_clicked();
    void on_btnPause_clicked();
    void on_btnResume_clicked();
    void on_btnStop_clicked();

private:
    Ui::MainWindow *ui;
    ProcessingController* controller;
};
