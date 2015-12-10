#ifndef TVMAINWINDOW_H
#define TVMAINWINDOW_H

#include <QMainWindow>
#include "tvgsrecorder.h"

namespace Ui {
class TVMainWindow;
}

class TVMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TVMainWindow(QWidget *parent = 0);
    ~TVMainWindow();

private slots:
    void on_startButton_clicked();

    void on_stopButton_clicked();

private:
    Ui::TVMainWindow *ui;
    TVGSRecorder *recorder;
};

#endif // TVMAINWINDOW_H
