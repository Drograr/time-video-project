// TimedVideo.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"
#include <Windows.h>

#include <gst/gst.h>

typedef struct _gst_elements {
	GstElement *rec_pipeline;
	GstElement *video_src, *tee, *queue, *sound_src, *video_enc, *sound_enc, *video_conv, *mux, *file_sink, *video_sink;
} gst_elements;

#ifdef __unix__
double linuxTime()
{
	long ms; // Milliseconds
	time_t s;  // Seconds
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);
	
	return (double) spec.tv_sec + (spec.tv_nsec * 1.0e-9);
}

#else
double win2LinuxTime(SYSTEMTIME st_time)
{
	FILETIME ft_time, ft_time1970;
	ULARGE_INTEGER ui_time, ui_time1970;
	SYSTEMTIME st_time1970;

	SystemTimeToFileTime(&st_time, &ft_time);
	ui_time.LowPart = ft_time.dwLowDateTime; ui_time.HighPart = ft_time.dwHighDateTime;
	st_time1970.wYear = 1970;
	st_time1970.wMonth = 1;
	st_time1970.wDay = 1;
	st_time1970.wHour = 0;
	st_time1970.wMinute = 0;
	st_time1970.wSecond = 0;
	st_time1970.wMilliseconds = 0;
	SystemTimeToFileTime(&st_time1970, &ft_time1970);
	ui_time1970.LowPart = ft_time1970.dwLowDateTime; ui_time1970.HighPart = ft_time1970.dwHighDateTime;
	return (double)(ui_time.QuadPart - ui_time1970.QuadPart) / 1e7;
}
#endif

static void newFrame_cb(GstPad *pad, GstPadProbeInfo *info, gpointer data)
{
#ifdef __unix__
	double time = linuxTime();
	g_print("FRAME,%f\n", time);
#else
	SYSTEMTIME currTime;
	GetLocalTime(&currTime);

	//Get current time
	g_print("FRAME,%d:%d:%d.%d,%f\n",
		currTime.wHour, currTime.wMinute, currTime.wSecond, currTime.wMilliseconds,
		win2LinuxTime(currTime));
#endif
}

static void state_gst_cb(GstBus *bus, GstMessage *msg, gst_elements *elts)
{
	if (GST_MESSAGE_SRC(msg) == GST_OBJECT(elts->rec_pipeline)) {
		GstState new_state;
		gst_message_parse_state_changed(msg, NULL, &new_state, NULL);
#ifdef __unix__
		double time = linuxTime();
		g_print("%s,%f\n", gst_element_state_get_name(new_state), time);
#else
		SYSTEMTIME currTime;
		GetLocalTime(&currTime);
		//Get current time
		g_print("%s,%d:%d:%d.%d,%f\n",
			gst_element_state_get_name(new_state),
			currTime.wHour, currTime.wMinute, currTime.wSecond, currTime.wMilliseconds,
			win2LinuxTime(currTime));
#endif
	}

}


static void error_gst_cb(GstBus *bus, GstMessage *msg, gst_elements *elts)
{
	GError *err;
	gchar* debug_info;

	gst_message_parse_error(msg, &err, &debug_info);
	g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
	g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
	g_clear_error(&err);
	g_free(debug_info);
}

static void eos_gst_cb(GstBus *bus, GstMessage *msg, GMainLoop *mainLoop)
{
	g_print("End-Of-Stream reached.\n");
	g_main_loop_quit(mainLoop);
}

GstElement* create_gst_element_err(const char* element, const char* name)
{
	GstElement *el;
	el = gst_element_factory_make(element, name);
	if(!el)
		g_printerr("Element %s of type %s could not be created.\n", name, element);
	return el;
}

static gst_elements *handlerParam;
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal + console closed (console close seems not to work)
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
			gst_element_send_event(handlerParam->sound_src, gst_event_new_eos());
			gst_element_send_event(handlerParam->video_src, gst_event_new_eos());
			return(TRUE);

		default:
			return FALSE;
	}
}

int main(int argc, char *argv[]) {
	gst_elements gst_elmts;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	gboolean terminate = FALSE;

	/* Initialize GStreamer */
	//gst_init(&argc, &argv);
	gst_init(NULL, NULL);

	if (argc != 2)
	{
		g_printerr("Parameter missing: must include the output file name (.mp4) as parameter, double \\\\ might be needed\n");
		return -1;
	}


	//Initialize a system clock
	GstClock *clock = gst_system_clock_obtain();
	g_object_set(clock, "clock-type", GST_CLOCK_TYPE_REALTIME, NULL);
	

	/* Create the elements*/
	gst_elmts.video_src = create_gst_element_err("ksvideosrc", "video_src");
	//video_src = create_gst_element_err("videotestsrc", "video_src");
	gst_elmts.tee = create_gst_element_err("tee", "tee");
	gst_elmts.queue = create_gst_element_err("queue", "queue");
	gst_elmts.sound_src = create_gst_element_err("directsoundsrc", "sound_src");
	gst_elmts.video_enc = create_gst_element_err("x264enc", "video_enc");
	//gst_elmts.video_enc = create_gst_element_err("openh264enc", "video_enc");
	gst_elmts.sound_enc = create_gst_element_err("flacenc", "sound_enc");
	gst_elmts.mux = create_gst_element_err("matroskamux", "mux");
	gst_elmts.file_sink = create_gst_element_err("filesink", "file_sink");
	gst_elmts.video_sink = create_gst_element_err("autovideosink", "video_sink");

	/* Create the pipelines */
	gst_elmts.rec_pipeline = gst_pipeline_new("rec_pipeline");
	if (!gst_elmts.rec_pipeline || !gst_elmts.queue || !gst_elmts.tee || !gst_elmts.video_src
		|| !gst_elmts.sound_src || !gst_elmts.video_enc || !gst_elmts.sound_enc || !gst_elmts.mux
		|| !gst_elmts.file_sink || !gst_elmts.video_sink)
		return -1;
	
	/* configure video source */
	/* configure sound source */
	/* configure video encoder */
	g_object_set(gst_elmts.video_enc,
//		"interlaced", TRUE,
		"pass", 4, //quant
		"quantizer", 18,
//		"pass", 0, //constant bitrate
//		"bitrate", 1024,
		"speed-preset", 2, //superfast
//		"byte-stream", TRUE,
		NULL);

	/*g_object_set(gst_elmts.video_enc,
		"complexity", 0,
		"biterate", 768000,
		NULL);*/

	/* configure output file */
	g_object_set(gst_elmts.file_sink, "location", argv[1], NULL);

	/* set the call back on the source to know when frames are coming in */
	GstPad *pad = gst_element_get_static_pad(gst_elmts.video_src, "src");
	gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) newFrame_cb, NULL, NULL);
	gst_object_unref(pad);

	/* Build the recording pipeline.	*/
	gst_bin_add_many(GST_BIN(gst_elmts.rec_pipeline), gst_elmts.video_src, gst_elmts.tee, gst_elmts.queue,
		gst_elmts.sound_src, gst_elmts.video_enc, gst_elmts.mux, gst_elmts.file_sink, gst_elmts.video_sink,
		gst_elmts.sound_enc, NULL);
	if (!gst_element_link_many(gst_elmts.video_src, gst_elmts.tee, gst_elmts.queue, gst_elmts.video_sink, NULL)
		|| !gst_element_link_many(gst_elmts.tee, gst_elmts.video_enc, gst_elmts.mux, gst_elmts.file_sink, NULL)
		|| !gst_element_link_many(gst_elmts.sound_src, gst_elmts.sound_enc, gst_elmts.mux, NULL)) {
		g_printerr("Elements could not be linked in rec pipeline.\n");
		gst_object_unref(gst_elmts.rec_pipeline);
		return -1;
	}

	/* Setup a main loop */
	GMainLoop *mainLoop = g_main_loop_new(NULL, FALSE);

	/* Listen to the bus and setup appropriate call back for messages */
	bus = gst_element_get_bus(gst_elmts.rec_pipeline);
	gst_bus_add_signal_watch(bus);
	g_signal_connect(G_OBJECT(bus), "message::error", (GCallback) error_gst_cb, mainLoop);
	g_signal_connect(G_OBJECT(bus), "message::eos", (GCallback) eos_gst_cb, mainLoop);
	g_signal_connect(G_OBJECT(bus), "message::state-changed", (GCallback)state_gst_cb, &gst_elmts);
	gst_object_unref(bus);

	/* Set handler for ctrl+c and console close */
	handlerParam = &gst_elmts; //to give data to the handler
	if(!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)) {
		g_printerr("Unable to set windows closing handlers.\n");
		gst_object_unref(gst_elmts.rec_pipeline);
		return -1;
	}


	/* Start playing */
	ret = gst_element_set_state(gst_elmts.rec_pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the rec_pipeline to the playing state.\n");
		gst_object_unref(gst_elmts.rec_pipeline);
		return -1;
	}

	//Enter loop up to an exit signal
	g_main_loop_run(mainLoop);

	/* Free resources */
	gst_element_set_state(gst_elmts.rec_pipeline, GST_STATE_NULL);
	gst_object_unref(gst_elmts.rec_pipeline);
	return 0;
}