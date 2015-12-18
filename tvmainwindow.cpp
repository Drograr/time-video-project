#include "tvmainwindow.h"
#include "ui_tvmainwindow.h"
#include <QSettings>

TVMainWindow::TVMainWindow(QWidget *parent, char* filename) :
    QMainWindow(parent),
    ui(new Ui::TVMainWindow)
{
    ui->setupUi(this);
    recorder = new TVGSRecorder(filename);

    //Connect handler to manage recorder started and finished events
    connect(recorder, SIGNAL(started()), this, SLOT(cb_recorder_started()));
    connect(recorder, SIGNAL(finished()), this, SLOT(cb_recorder_finished()));

    //Load last user settings
    QSettings settings(QApplication::applicationDirPath() + "/TimedVideo.ini", QSettings::IniFormat);
    int videoQuantizerSpinBox= settings.value("video/quantizer",18).toInt();
    int videoSpeedSpinBox = settings.value("video/speed", 1).toInt();
    int audioQualitySpinBox = settings.value("audio/quality", 2).toInt();
    ui->videoQuantizerSpinBox->setValue(videoQuantizerSpinBox);
    ui->videoSpeedSpinBox->setValue(videoSpeedSpinBox);
    ui->audioQualitySpinBox->setValue(audioQualitySpinBox);
}

TVMainWindow::~TVMainWindow()
{
    //Save User Settings
    QSettings settings(QApplication::applicationDirPath() + "/TimedVideo.ini", QSettings::IniFormat);
    settings.setValue("video/quantizer",ui->videoQuantizerSpinBox->value());
    settings.setValue("video/speed",ui->videoSpeedSpinBox->value());
    settings.setValue("audio/quality",ui->audioQualitySpinBox->value());

    //Clean up the class member variables
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

