#include "tvmainwindow.h"
#include "ui_tvmainwindow.h"
#include <QTimer>

TVMainWindow::TVMainWindow(QWidget *parent, char* filename) :
    QMainWindow(parent),
    ui(new Ui::TVMainWindow)
{
    ui->setupUi(this);
    recorder = new TVGSRecorder(filename);

    //Connect handler to manage recorder started and finished events
    connect(recorder, SIGNAL(started()), this, SLOT(cb_recorder_started()));
    connect(recorder, SIGNAL(finished()), this, SLOT(cb_recorder_finished()));
}

TVMainWindow::~TVMainWindow()
{
    delete ui;
    delete recorder;
}

void TVMainWindow::on_startButton_clicked()
{
    //Initialize the recording player if needed
    if(recorder->init_pipeline(ui->videoQuantizerSpinBox->value(), ui->videoSpeedSpinBox->value(), ui->audioQualitySpinBox->value()) == false)
        return;

    //Associate recorder video output the video widget (should be done each time, why ?)
    recorder->setDisplay(ui->videoWidget);

    //Start playing
    recorder->start();
}

void TVMainWindow::on_stopButton_clicked()
{
    recorder->stop();
}

void TVMainWindow::cb_recorder_started()
{
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
}

void TVMainWindow::cb_recorder_finished()
{
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void TVMainWindow::closeEvent (QCloseEvent *event)
{
    recorder->stop();
    recorder->wait();
}

