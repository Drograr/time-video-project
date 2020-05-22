#ifndef TVGSRECORDER_H
#define TVGSRECORDER_H

#include <gst/gst.h>
#include <QThread>

typedef struct Resolutions{
	char hauteur[5];
	char largeur[5];
	char framerate[6];
	int framerateUP;
	int framerateDOWN;
	}Resolution;

typedef struct Cameras{
	char path[200];
	Resolution options[2000];
	char nom[200];
	unsigned int nbr_resolution;
	}Camera;

class TVGSRecorder : public QThread
{
public:

    TVGSRecorder(gchar* _filename);
    ~TVGSRecorder();
    bool init_pipeline(int videoFrameRateUP, int videoFrameRateDOWN, QSize videoSize, char videoQuantizer, char videoSpeedPreset, char audioQuality,char *path);
    void destroy_pipeline();
    void stop();
    void setDisplay(QWidget *widget);
    bool isPlaying();

private:
    GstElement *rec_pipeline;
    GstElement *video_src,*tee, *queue, *sound_src, *video_enc, *sound_enc, *video_conv, *mux, *file_sink, *video_sink;
    GstBus *bus;
    gchar *filename;
    GMainLoop *mainLoop;
    GstState currState;
    bool pipelineInitialized;

    void run();

    GstElement* create_gst_element_err(const char* element, const char* name);
    void state_gst(GstMessage *msg);
    void error_gst(GstMessage *msg);
    void eos_gst();

    static void newFrame_cb(GstPad *pad, GstPadProbeInfo *info, gpointer unused);
};

#endif // TVGSRECORDER_H
