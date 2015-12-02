// TimedVideo.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"


#include <gst/gst.h>
#include <Windows.h>


/* Functions below print the Capabilities in a human-friendly format */
static gboolean print_field(GQuark field, const GValue * value, gpointer pfx) {
	gchar *str = gst_value_serialize(value);

	g_print("%s  %15s: %s\n", (gchar *)pfx, g_quark_to_string(field), str);
	g_free(str);
	return TRUE;
}

static void print_caps(const GstCaps * caps, const gchar * pfx) {
	guint i;

	g_return_if_fail(caps != NULL);

	if (gst_caps_is_any(caps)) {
		g_print("%sANY\n", pfx);
		return;
	}
	if (gst_caps_is_empty(caps)) {
		g_print("%sEMPTY\n", pfx);
		return;
	}

	for (i = 0; i < gst_caps_get_size(caps); i++) {
		GstStructure *structure = gst_caps_get_structure(caps, i);

		g_print("%s%s\n", pfx, gst_structure_get_name(structure));
		gst_structure_foreach(structure, print_field, (gpointer)pfx);
	}
}

/* Shows the CURRENT capabilities of the requested pad in the given element */
static void print_pad_capabilities(GstElement *element, gchar *pad_name) {
	GstPad *pad = NULL;
	GstCaps *caps = NULL;

	/* Retrieve pad */
	pad = gst_element_get_static_pad(element, pad_name);
	if (!pad) {
		g_printerr("Could not retrieve pad '%s'\n", pad_name);
		return;
	}

	/* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) */
	caps = gst_pad_get_current_caps(pad);
	if (!caps)
	{
		g_printerr("No caps currently configured\n");
		return;
	}

	/* Print and free */
	g_print("Caps for the %s pad:\n", pad_name);
	print_caps(caps, "      ");
	gst_caps_unref(caps);
	gst_object_unref(pad);
}


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

static void newFrame_cb(GstPad *pad, GstPadProbeInfo *info, gpointer data)
{
	SYSTEMTIME currTime;
	GetLocalTime(&currTime);

	//Get current time
	g_print("FRAME,%d:%d:%d.%d,%f\n",
		currTime.wHour, currTime.wMinute, currTime.wSecond, currTime.wMilliseconds,
		win2LinuxTime(currTime));
}

int main(int argc, char *argv[]) {
	GstElement *pipeline, *video_src, *sound_src, *video_enc, *video_conv, *mux, *file_sink, *video_sink;
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
	

	/* Create the elements */
	video_src = gst_element_factory_make("ksvideosrc", "video_src");
	//video_src = gst_element_factory_make("videotestsrc", "video_src");
	sound_src = gst_element_factory_make("directsoundsrc", "sound_src");
	video_enc = gst_element_factory_make("x264enc", "video_enc");
	mux = gst_element_factory_make("matroskamux", "mux");
	file_sink = gst_element_factory_make("filesink", "file_sink");
	g_object_set(file_sink, "location", argv[1], NULL);
	video_sink = gst_element_factory_make("autovideosink", "video_sink");

	/* Create the empty pipeline */
	pipeline = gst_pipeline_new("test-pipeline");
	if (!pipeline || !video_src || !sound_src || !video_enc || !mux || !file_sink || !video_sink) {
		g_printerr("Not all elements could be created.\n");
		return -1;
	}

	/* set the call back on the source to know when frames are coming in */
	GstPad *pad = gst_element_get_static_pad(video_src, "src");
	gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) newFrame_cb, NULL, NULL);
	gst_object_unref(pad);

	/* Build the pipeline.
	*/
	gst_bin_add_many(GST_BIN(pipeline), video_src, sound_src, video_enc, mux, file_sink, /*video_sink,*/ NULL);
	if (!gst_element_link_many(video_src, video_enc, mux, file_sink, NULL) || !gst_element_link(sound_src, mux)) {
		g_printerr("Elements could not be linked.\n");
		gst_object_unref(pipeline);
		return -1;
	}

	/* Start playing */
	ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(pipeline);
		return -1;
	}

	/* Listen to the bus */
	bus = gst_element_get_bus(pipeline);
	do {
		msg = gst_bus_timed_pop_filtered(bus, /*GST_CLOCK_TIME_NONE*/ 60000*GST_MSECOND,
			(GstMessageType) (GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_BUFFERING));

		/* Parse message */
		if (msg != NULL) {
			GError *err;
			gchar *debug_info;

			switch (GST_MESSAGE_TYPE(msg)) {
			case GST_MESSAGE_ERROR:
				gst_message_parse_error(msg, &err, &debug_info);
				g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
				g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
				g_clear_error(&err);
				g_free(debug_info);
				terminate = TRUE;
				break;
			case GST_MESSAGE_EOS:
				g_print("End-Of-Stream reached.\n");
				terminate = TRUE;
				break;
			case GST_MESSAGE_STATE_CHANGED:				
				if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline)) {
					GstState new_state;
					gst_message_parse_state_changed(msg, NULL, &new_state, NULL);
					SYSTEMTIME currTime;
					GetLocalTime(&currTime);

					//Get current time
					g_print("%s,%d:%d:%d.%d,%f\n",
						gst_element_state_get_name(new_state),
						currTime.wHour, currTime.wMinute, currTime.wSecond, currTime.wMilliseconds,
						win2LinuxTime(currTime));
				}
				break;
			case GST_MESSAGE_BUFFERING:
				SYSTEMTIME currTime;
				GetLocalTime(&currTime);

				//Get current time
				g_print("BUFFERING,%d:%d:%d.%d,%f\n",
					currTime.wHour, currTime.wMinute, currTime.wSecond, currTime.wMilliseconds,
					win2LinuxTime(currTime));
				break;
			default:
				/* We should not reach here */
				g_printerr("Unexpected message received.\n");
				break;
			}
			gst_message_unref(msg);
		}
		else
		{
			GstState state;
			gst_element_get_state(pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
			if (state == GST_STATE_PLAYING)
				terminate = TRUE;
			
		}
	} while (!terminate);

	/* Free resources */
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	return 0;
}