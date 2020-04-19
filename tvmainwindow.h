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
    explicit TVMainWindow(QWidget *parent = 0, char *filename = 0);
    ~TVMainWindow();
    Camera liste_cameras[50];
    int nbr_cameras;
    void camera_caps();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void cb_recorder_started();
    void cb_recorder_finished();
    void UpdateCombo();

private:
    Ui::TVMainWindow *ui;
    TVGSRecorder *recorder;
    char* filename;
    void closeEvent (QCloseEvent *event);
    
	
};

#endif // TVMAINWINDOW_H
