#include <QtGlobal>
#include <QTime>
#include <QWidget>
#include "tvgsrecorder.h"
#include <gst/video/videooverlay.h>
void* __gxx_personality_v0; //This is a very crapy hack because otherwise it complains

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
    currState = GST_STATE_NULL;
    pipelineInitialized = false;
}

TVGSRecorder::~TVGSRecorder()
{
    destroy_pipeline();
}


bool TVGSRecorder::init_pipeline(char videoQuantizer, char videoSpeedPreset, char audioQuality)
{

    //Destroy the previous pipeline
    destroy_pipeline();

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

#ifdef Q_OS_WIN32
    video_sink = create_gst_element_err("d3dvideosink", "video_sink");
#elif defined(Q_OS_LINUX)
    video_sink = create_gst_element_err("xvimagesink", "video_sink");
#else
#error "Your platform is not supported"
#endif


    /* Create the pipeline */
    rec_pipeline = gst_pipeline_new("rec_pipeline");

    /* If any element failed to initilizecurrState == GST_STATE_PLAYING return false */
    if (!rec_pipeline || !queue || !tee || !video_src || !sound_src || !video_enc
            || !sound_enc || !mux   || !file_sink || !video_sink)
        return false;

    /* configure video source */
    /* configure sound source */
    /* configure video encoder */
    g_object_set(video_enc,
        "pass", 4, //use quantizer
        "quantizer", videoQuantizer,
        "speed-preset",  videoSpeedPreset,
        NULL);

    /* Configure sound encoding */
    g_object_set(sound_enc, "quality", audioQuality, NULL);

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


    pipelineInitialized = true;
    return true;

}

void TVGSRecorder::destroy_pipeline()
{

    if(pipelineInitialized)
    {
        gst_object_unref(rec_pipeline);
        pipelineInitialized = false;
    }
}




void TVGSRecorder::run(void)
{

    /* If the recorder is already in playing state do not start recording */
    if(isPlaying() || !pipelineInitialized)
        return;

    /* Start playing */
    GstStateChangeReturn ret = gst_element_set_state(rec_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the rec_pipeline to the playing state.\n");
        gst_object_unref(rec_pipeline);
        pipelineInitialized = false;
        return;
    }

    bus = gst_element_get_bus(rec_pipeline);
    GstMessage *msg;
    bool terminate;
    do {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE /*100*GST_MSECOND*/,
            (GstMessageType) (GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_BUFFERING));

        /* Parse message */
        if (msg != NULL) {
            switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR:
                error_gst(msg);
                terminate = true;
                break;
            case GST_MESSAGE_EOS:
                eos_gst();
                terminate = true;
                break;
            case GST_MESSAGE_STATE_CHANGED:
                state_gst(msg);
                break;
            default:
                /* We should not reach here */
                g_printerr("Unexpected message received.\n");
                break;
            }
            gst_message_unref(msg);
        }
    } while (!terminate);

    /* Free resources, TODO: Put somewhere else */
    gst_object_unref(bus);
    gst_element_set_state(rec_pipeline, GST_STATE_NULL);
    currState = GST_STATE_NULL; //Not very good to do it manually...
}

void TVGSRecorder::stop(void)
{
    if(isPlaying())
    {
        gst_element_send_event(sound_src, gst_event_new_eos());
        gst_element_send_event(video_src, gst_event_new_eos());
    }
}

void TVGSRecorder::setDisplay(QWidget *widget)
{
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(video_sink), widget->winId());
}

bool TVGSRecorder::isPlaying()
{
    if(currState == GST_STATE_PLAYING)
        return true;
    else
        return false;
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

    g_print("FRAME,%d:%d:%d.%d,%.3f\n",
        localTime.hour(), localTime.minute(), localTime.second(), localTime.msec(),
        localDate.toTime_t() + (localTime.msec() * 1e-3));
}

void TVGSRecorder::state_gst(GstMessage *msg)
{
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(rec_pipeline)) {
        gst_message_parse_state_changed(msg, NULL, &currState, NULL);

        //Get current time
        QDateTime localDate(QDateTime::currentDateTime());
        QTime  localTime = localDate.time();

        g_print("%s,%d:%d:%d.%d,%.3f\n",
            gst_element_state_get_name(currState),
            localTime.hour(), localTime.minute(), localTime.second(), localTime.msec(),
            localDate.toTime_t() + (localTime.msec() * 1e-3));
    }
}

void TVGSRecorder::error_gst(GstMessage *msg)
{
    GError *err;
    gchar* debug_info;

    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);
}

void TVGSRecorder::eos_gst()
{
    g_print("End-Of-Stream reached.\n");
}
