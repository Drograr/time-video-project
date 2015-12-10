#ifndef TVGSRECORDER_H
#define TVGSRECORDER_H

#include <gst/gst.h>

class TVGSRecorder
{
public:
    TVGSRecorder(gchar* _filename);
    bool init_pipeline();
    bool start();
    void stop();

private:
    GstElement *rec_pipeline;
    GstElement *video_src,*tee, *queue, *sound_src, *video_enc, *sound_enc, *video_conv, *mux, *file_sink, *video_sink;
    GstBus *bus;
    gchar *filename;
    static GMainLoop *mainLoop;

    GstElement* create_gst_element_err(const char* element, const char* name);
    static void newFrame_cb(GstPad *pad, GstPadProbeInfo *info, gpointer unused);
    static void state_gst_cb(GstBus *bus, GstMessage *msg, gpointer unused);
    static void error_gst_cb(GstBus *bus, GstMessage *msg, gpointer unused);
    static void eos_gst_cb(GstBus *bus, GstMessage *msg, gpointer unused);
};

#endif // TVGSRECORDER_H
