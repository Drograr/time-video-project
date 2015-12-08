#include "tvmainwindow.h"
#include "ui_tvmainwindow.h"

TVMainWindow::TVMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TVMainWindow)
{
    ui->setupUi(this);
}

TVMainWindow::~TVMainWindow()
{
    delete ui;
}
