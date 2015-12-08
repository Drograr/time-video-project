#ifndef TVMAINWINDOW_H
#define TVMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class TVMainWindow;
}

class TVMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TVMainWindow(QWidget *parent = 0);
    ~TVMainWindow();

private:
    Ui::TVMainWindow *ui;
};

#endif // TVMAINWINDOW_H
