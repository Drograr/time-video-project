#include "tvmainwindow.h"
#include <QApplication>
#include <gst/gst.h>

//TODO: BAAAAAD !
//#include "timedvideo.cpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* Initialize GStreamer */
    //gst_init(&argc, &argv);
    gst_init(NULL, NULL);

    if (argc != 2)
    {
        g_printerr("Parameter missing: must include the output file name (.mp4) as parameter, double \\\\ might be needed\n");
        return -1;
    }


    TVMainWindow w;
    w.show();

    //main_TV(argc, argv);

    return a.exec();
}
