#include "tvmainwindow.h"
#include <QApplication>
#include <gst/gst.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* Initialize GStreamer */
    //gst_init(&argc, &argv);
    gst_init(NULL, NULL);

    if (argc != 2)
    {
        qFatal("Parameter missing: must include the output file name (.mp4) as parameter, double \\\\ might be needed\n");
        return -1;
    }

    TVMainWindow w(NULL,  argv[1]);
    w.show();

    return a.exec();
}
