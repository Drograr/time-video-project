#include "tvmainwindow.h"
#include <QApplication>


//TODO: BAAAAAD !
#include "timedvideo.cpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TVMainWindow w;
    w.show();

    main_TV(argc, argv);

    return a.exec();
}
