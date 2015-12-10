#ifndef TVGSRECORDER_H
#define TVGSRECORDER_H

#include <gst/gst.h>
#include <QThread>

class TVGSRecorder : public QThread
{
public:
    TVGSRecorder(gchar* _filename);
    ~TVGSRecorder();
    bool init_pipeline();
    void stop();

private:
    GstElement *rec_pipeline;
    GstElement *video_src,*tee, *queue, *sound_src, *video_enc, *sound_enc, *video_conv, *mux, *file_sink, *video_sink;
    GstBus *bus;
    gchar *filename;
    GMainLoop *mainLoop;
    GstState currState;

    void run();

    GstElement* create_gst_element_err(const char* element, const char* name);
    void state_gst(GstMessage *msg);
    void error_gst(GstMessage *msg);
    void eos_gst();

    static void newFrame_cb(GstPad *pad, GstPadProbeInfo *info, gpointer unused);
};

#endif // TVGSRECORDER_H
