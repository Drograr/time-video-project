#include "tvmainwindow.h"
#include "ui_tvmainwindow.h"
#include <QTimer>

TVMainWindow::TVMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TVMainWindow)
{
    ui->setupUi(this);
    recorder = NULL;
}

TVMainWindow::~TVMainWindow()
{
    delete ui;
}

void TVMainWindow::on_startButton_clicked()
{
    //Initialize the recording player if needed
    if(recorder == NULL)
    {
        recorder = new TVGSRecorder("test.mp4");
        if(recorder->init_pipeline() == false)
            close();
    }

    //Start playing
    recorder->start();
}

void TVMainWindow::on_stopButton_clicked()
{
    recorder->stop();
}
