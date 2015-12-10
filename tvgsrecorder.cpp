#include <QtGlobal>
#include <QTime>
#include "tvgsrecorder.h"

TVGSRecorder::TVGSRecorder(gchar* _filename)
{
    //TODO: Copy content instead of pointers ?
    filename = _filename;

    rec_pipeline = NULL;
    video_src = NULL;
    tee = NULL;
    queue = NULL;
    sound_src = NULL;
    video_enc = NULL;
    sound_enc = NULL;
    video_conv = NULL;
    mux = NULL;
    file_sink = NULL;
    video_sink = NULL;
    bus = NULL;
}

bool TVGSRecorder::init_pipeline()
{
    /* Create the elements*/
#ifdef Q_OS_WIN32
    video_src = create_gst_element_err("ksvideosrc", "video_src");
#elif defined(Q_OS_LINUX)
    video_src = create_gst_element_err("v4l2src", "video_src");
#else
#error "Your platform is not supported"
#endif
    //video_src = create_gst_element_err("videotestsrc", "video_src");
    tee = create_gst_element_err("tee", "tee");

    queue = create_gst_element_err("queue", "queue");

#ifdef Q_OS_WIN32
    sound_src = create_gst_element_err("directsoundsrc", "sound_src");
#elif defined(Q_OS_LINUX)
    sound_src = create_gst_element_err("alsasrc", "sound_src");
#else
#error "Your platform is not supported"
#endif

    //gst_elmts.video_enc = create_gst_element_err("mpeg2enc", "video_enc");
    video_enc = create_gst_element_err("x264enc", "video_enc");
    //gst_elmts.video_enc = create_gst_element_err("openh264enc", "video_enc");

    sound_enc = create_gst_element_err("flacenc", "sound_enc");

    mux = create_gst_element_err("matroskamux", "mux");
    //gst_elmts.mux = create_gst_element_err("avimux", "mux");

    file_sink = create_gst_element_err("filesink", "file_sink");

    video_sink = create_gst_element_err("autovideosink", "video_sink");

    /* Create the pipeline */
    rec_pipeline = gst_pipeline_new("rec_pipeline");

    /* If any element failed to initilize return false */
    if (!rec_pipeline || !queue || !tee || !video_src || !sound_src || !video_enc
            || !sound_enc || !mux   || !file_sink || !video_sink)
        return false;

    /* configure video source */
    /* configure sound source */
    /* configure video encoder */
    g_object_set(video_enc,
//		"interlaced", TRUE,
        "pass", 4, //quant
        "quantizer", 25,
//        "pass", 0, //constant bitrate
//        "bitrate", 1024,
        "speed-preset", 1, //superfast
//		"byte-stream", TRUE,
        NULL);

    /*g_object_set(gst_elmts.video_enc,
        "complexity", 0,
        "biterate", 768000,
        NULL);*/

    /* configure output file */
    g_object_set(file_sink, "location", filename, NULL);

    /* set the call back on the source to know when frames are coming in */
    GstPad *pad = gst_element_get_static_pad(video_src, "src");
    gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) newFrame_cb, NULL, NULL);
    gst_object_unref(pad);

    /* Build the recording pipeline. */
    gst_bin_add_many(GST_BIN(rec_pipeline), video_src, tee, queue, sound_src, video_enc, mux,
                     file_sink, video_sink, sound_enc, NULL);

    GstCaps *capsFilter = gst_caps_new_simple("video/x-raw",
                                        "width", G_TYPE_INT, 640,
                                        "height", G_TYPE_INT, 480,
                                        "framerate", GST_TYPE_FRACTION, 30, 1,
                                        NULL);


    if(!gst_element_link_filtered(video_src, tee, capsFilter))
    {
        g_printerr("Filtered elements could not be linked in rec pipeline.\n");
        gst_caps_unref(capsFilter);
        gst_object_unref(rec_pipeline);
        return false;
    }
    gst_caps_unref(capsFilter);

    if(!gst_element_link_many(tee, queue, video_sink, NULL) || !gst_element_link_many(tee, video_enc, mux, file_sink, NULL)
        || !gst_element_link_many(sound_src, sound_enc, mux, NULL))
    {
        g_printerr("Elements could not be linked in rec pipeline.\n");
        gst_object_unref(rec_pipeline);
        return false;
    }

    /* Setup a main loop */
    mainLoop = g_main_loop_new(NULL, FALSE);

    /* Listen to the bus and setup appropriate call back for messages */
    bus = gst_element_get_bus(rec_pipeline);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(G_OBJECT(bus), "message::error", (GCallback) error_gst_cb, NULL);
    g_signal_connect(G_OBJECT(bus), "message::eos", (GCallback) eos_gst_cb, NULL);
    g_signal_connect(G_OBJECT(bus), "message::state-changed", (GCallback)state_gst_cb, NULL);
    gst_object_unref(bus);



    return true;

}

bool TVGSRecorder::start(void)
{
    /* Start playing */
    GstStateChangeReturn ret = gst_element_set_state(rec_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the rec_pipeline to the playing state.\n");
        gst_object_unref(rec_pipeline);
        return false;
    }

    //Enter loop up to an exit signal
    g_main_loop_run(mainLoop);

    /* Free resources, TODO: Put somewhere else */
    gst_element_set_state(rec_pipeline, GST_STATE_NULL);
    gst_object_unref(rec_pipeline);

    return true;

}

void TVGSRecorder::stop(void)
{
    gst_element_send_event(sound_src, gst_event_new_eos());
    gst_element_send_event(video_src, gst_event_new_eos());
}

GstElement* TVGSRecorder::create_gst_element_err(const char* element, const char* name)
{
    GstElement *el;
    el = gst_element_factory_make(element, name);
    if(!el)
        g_printerr("Element %s of type %s could not be created.\n", name, element);
    return el;
}

void TVGSRecorder::newFrame_cb(GstPad *pad, GstPadProbeInfo *info, gpointer unused)
{
    //Get current time
    QDateTime localDate(QDateTime::currentDateTime());
    QTime  localTime = localDate.time();

    g_print("FRAME,%d:%d:%d.%d,%f\n",
        localTime.hour(), localTime.minute(), localTime.second(), localTime.msec(),
        localDate.toTime_t() + (localTime.msec() / 1e-3));
}

void TVGSRecorder::state_gst_cb(GstBus *bus, GstMessage *msg, gpointer unused)
{
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(rec_pipeline)) {
        GstState new_state;
        gst_message_parse_state_changed(msg, NULL, &new_state, NULL);

        //Get current time
        QDateTime localDate(QDateTime::currentDateTime());
        QTime  localTime = localDate.time();

        g_print("%s,%d:%d:%d.%d,%f\n",
            gst_element_state_get_name(new_state),
            localTime.hour(), localTime.minute(), localTime.second(), localTime.msec(),
            localDate.toTime_t() + (localTime.msec() / 1e-3));
    }
}

void TVGSRecorder::error_gst_cb(GstBus *bus, GstMessage *msg, gpointer unused)
{
    GError *err;
    gchar* debug_info;

    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);
}

void TVGSRecorder::eos_gst_cb(GstBus *bus, GstMessage *msg, gpointer unused)
{
    g_print("End-Of-Stream reached.\n");
    g_main_loop_quit(mainLoop);
}
