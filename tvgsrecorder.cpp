#include <QtGlobal>
#include <QTime>
#include <QWidget>
#include <QSize>
#include <gst/video/videooverlay.h>

#include "tvgsrecorder.h"


//Define the GST elements which depends on the platform
#ifdef Q_OS_WIN32
    #define VIDEO_SRC "ksvideosrc"
    #define SOUND_SRC "directsoundsrc"
    #define VIDEO_SINK "d3dvideosink"
#elif defined(Q_OS_LINUX)
	#include <unistd.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <string.h>
	#include <stdlib.h>
	#include <sys/wait.h> 

    #define VIDEO_SRC "v4l2src"
    #define SOUND_SRC "alsasrc"
    #define VIDEO_SINK "xvimagesink"
#else
    #error "Your platform is not supported"
#endif

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


bool TVGSRecorder::init_pipeline(QString videoFrameRate, QSize videoSize, char videoQuantizer, char videoSpeedPreset, char audioQuality)
{

    //Destroy the previous pipeline
    destroy_pipeline();

    /* Create the elements*/
    // Video source
    video_src = create_gst_element_err(VIDEO_SRC, "video_src");
    // Tee to split video in two branches: recording and display
    tee = create_gst_element_err("tee", "tee");
    // Queue to create new thread for video recording
    queue = create_gst_element_err("queue", "queue");
    // Sound source
    sound_src = create_gst_element_err(SOUND_SRC, "sound_src");
    // Video encoder for h264 using x264 encoder
    video_enc = create_gst_element_err("x264enc", "video_enc");
    // Helps to improve video source compatibility with encoder
    video_conv = create_gst_element_err("videoconvert", "video_conv");
    // Encode sound in FLAC lossless
    sound_enc = create_gst_element_err("flacenc", "sound_enc");
    // Mux sound and video in matroska format
    mux = create_gst_element_err("matroskamux", "mux");
    // Record to file
    file_sink = create_gst_element_err("filesink", "file_sink");
    // Display video for feedback
    video_sink = create_gst_element_err(VIDEO_SINK, "video_sink");

    /* Create the pipeline */
    rec_pipeline = gst_pipeline_new("rec_pipeline");

    /* If any element failed to initilizecurrState == GST_STATE_PLAYING return false */
    if (!rec_pipeline || !queue || !tee || !video_src || !sound_src || !video_enc
            || !video_conv || !sound_enc || !mux   || !file_sink || !video_sink)
        return false;

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

    /* Create the bin and continue by building pipeline */
    gst_bin_add_many(GST_BIN(rec_pipeline), video_src, tee, queue, sound_src, video_enc, mux,
                     video_conv, file_sink, video_sink, sound_enc, NULL);

    /* Link the video source with other components by filtering the resolution and framerate */
    GstCaps *capsFilter = gst_caps_new_simple("video/x-raw",
                                        "width", G_TYPE_INT, videoSize.width(),
                                        "height", G_TYPE_INT, videoSize.height(),
                                        "framerate", GST_TYPE_FRACTION, videoFrameRate, 1,
                                        NULL);
    if(!gst_element_link_filtered(video_src, tee, capsFilter))
    {
        g_printerr("Filtered elements could not be linked in rec pipeline.\n");
        gst_caps_unref(capsFilter);
        gst_object_unref(rec_pipeline);
        return false;
    }
    gst_caps_unref(capsFilter);

    /* create two video branches and one sound branch :
     * - one with the video encoding / recording
     * - the other with the video display
     * - the last for sound recording */
    if(!gst_element_link_many(tee, queue, video_sink, NULL) || !gst_element_link_many(tee, video_conv, video_enc, mux, file_sink, NULL)
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

    /* Initialize and run the main event loop for gstreamer */
    bus = gst_element_get_bus(rec_pipeline);
    GstMessage *msg;
    bool terminate = false;
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

    /* Free resources */
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

void TVGSRecorder::camera_caps()
{
	FILE *file_parsed = fopen("./test.txt","r");
	int compteur_fgets = 0,mode_parseur = 0,buff_compteur_fgets = 0;
	int compteur_strtok_caps = 0;
	char *token;
	char *token_caps;
	char string_container[1000];
	char nom_device[1000]; 
	char type_device_video[1000];
	char buff_caracteristique[1000];
	int flag_mode4 = 0;
	char *context_parse;
	char *context_caps;
	int buff_compteur_path;
	int compteur_resolution_buffeur;
	char *token_precedent;
	
	
	
	int compteur_premier_framerate = 0;
	int compteur_resolution = -1;
	
	int compteur_device = -1;
	char tableau_nom[50][200];
	char buffeur_filtre[200];
	char *token_filtre;
	char *token_filtre2;
	char *context_filtre;
	
	
	pid_t pid;
	int msg_waitpid;
	
	pid = fork();
	if (pid == 0) {
		int output_gst_device_monitor = open("./test.txt", O_TRUNC | O_WRONLY | O_CREAT, S_IWUSR|S_IWGRP| S_IRUSR | S_IRGRP );
		dup2(output_gst_device_monitor, 1);
		execvp("gst-device-monitor-1.0",NULL);
		}
	else{
		waitpid(pid,&msg_waitpid,0);
		}
	
	
	
	
	
	while (fgets(string_container, 1000, file_parsed) != NULL){
		compteur_fgets += 1;
		token = strtok_r(string_container," ",&context_parse);
		while (token != NULL){
			
			compteur_fgets += 1;
			
			
			if (mode_parseur == 0)
			//trouve le premier device
			{if (strcmp("Device",token) == 0)
				{   buff_compteur_fgets = compteur_fgets;
					mode_parseur = 1;
					
					}
				}
			if (mode_parseur == 1){
				//trouve le nom de Device
				if (compteur_fgets == buff_compteur_fgets + 6){
					mode_parseur = 2;
					}
				}
			if (mode_parseur == 2){
				//  stocke le nom du Device
				strcpy(nom_device,token);
				
				
				mode_parseur = 3;
				}
			if (mode_parseur == 3){
				//trouve l'indiquateur du fait qu'on se trouve sur une camera ou bien 
				//de l'audio'
				if (compteur_fgets == buff_compteur_fgets + 9){
					mode_parseur = 4;
					}
				}
				
			if (mode_parseur == 4){
				//verifie qu'on soit bien sur une source vidéo et si c'est le cas on met son nom dans la liste
				
				strcpy(type_device_video,token);
				
				if(strcmp(": Video/Source\n",type_device_video) != 0){
					buff_compteur_fgets = 0;
					mode_parseur = 0;
					}
				else{
					buff_compteur_fgets = compteur_fgets;
					flag_mode4 = 1;
					mode_parseur = 5 ;
					compteur_device += 1;
					compteur_resolution = -1;
					strcpy(tableau_nom[compteur_device],nom_device);
					
					}
				}
			if (mode_parseur == 5){//trouve les resolution/framerate et les mets dans un format utilisable.
				if (((buff_compteur_fgets - compteur_fgets) % 3) == 0){
					
					strcpy(buff_caracteristique,token);
					
					
					if(flag_mode4 ==1){
						flag_mode4 = 0;}
					else {
						
					if (buff_caracteristique[1] == ':') {
						memmove(buff_caracteristique, buff_caracteristique+3, strlen(buff_caracteristique));
						
						}
					else {
						memmove(buff_caracteristique, buff_caracteristique+7, strlen(buff_caracteristique));
												
						}
					
					compteur_strtok_caps = 0;
					token_caps = strtok_r(buff_caracteristique,",",&context_caps);
					while(token_caps != NULL){
						if (compteur_strtok_caps == 0){
							if (strcmp(token_caps,"video/x-raw") != 0){
								break;}
							
							else{
								
								}}
						if (compteur_strtok_caps != 0){
							memmove(token_caps, token_caps+1, strlen(token_caps));
							}
						
						if (compteur_strtok_caps == 2){
							compteur_resolution_buffeur = compteur_resolution;
							if (token_caps[14] >= '0' && token_caps[14] <= '9'){
								
								liste_cameras[compteur_device].options[compteur_resolution].largeur[0] = token_caps[11];
								liste_cameras[compteur_device].options[compteur_resolution].largeur[1] = token_caps[12];
								liste_cameras[compteur_device].options[compteur_resolution].largeur[2] = token_caps[13];
								liste_cameras[compteur_device].options[compteur_resolution].largeur[3] = token_caps[14];
								liste_cameras[compteur_device].options[compteur_resolution].largeur[4] = '\0';
								}
							else{
								liste_cameras[compteur_device].options[compteur_resolution].largeur[0] = token_caps[11];
								liste_cameras[compteur_device].options[compteur_resolution].largeur[1] = token_caps[12];
								liste_cameras[compteur_device].options[compteur_resolution].largeur[2] = token_caps[13];
								liste_cameras[compteur_device].options[compteur_resolution].largeur[3] = '\0';
								}
							}
						if (compteur_strtok_caps == 3){
							if (token_caps[15] >= '0' && token_caps[15] <= '9'){
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[0] = token_caps[12];
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[1] = token_caps[13];
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[2] = token_caps[14];
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[3] = token_caps[15];
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[4] = '\0';
								}
							else{
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[0] = token_caps[12];
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[1] = token_caps[13];
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[2] = token_caps[14];
								liste_cameras[compteur_device].options[compteur_resolution].hauteur[3] = '\0';
								}
							}
							
						if (compteur_strtok_caps == 5){
							compteur_premier_framerate = 0;
							while (token_caps[20+compteur_premier_framerate] != '/'){
								liste_cameras[compteur_device].options[compteur_resolution].framerate[compteur_premier_framerate] = token_caps[20+compteur_premier_framerate];
								
								compteur_premier_framerate += 1;
								
								}
								liste_cameras[compteur_device].options[compteur_resolution].framerate[compteur_premier_framerate] = token_caps[20+compteur_premier_framerate];
								compteur_premier_framerate += 1;
								liste_cameras[compteur_device].options[compteur_resolution].framerate[compteur_premier_framerate] = token_caps[20+compteur_premier_framerate];
								compteur_premier_framerate += 1;
								liste_cameras[compteur_device].options[compteur_resolution].framerate[compteur_premier_framerate] = '\0';
								
							compteur_resolution++;
							}
						if (compteur_strtok_caps > 5){
							strcpy(liste_cameras[compteur_device].options[compteur_resolution].hauteur,liste_cameras[compteur_device].options[compteur_resolution_buffeur].hauteur);
							strcpy(liste_cameras[compteur_device].options[compteur_resolution].largeur,liste_cameras[compteur_device].options[compteur_resolution_buffeur].largeur);
							
							strcpy(buffeur_filtre,token_caps);
							token_filtre = strtok_r(buffeur_filtre,"}",&context_filtre);
							token_filtre2 = strtok_r(NULL,"}",&context_filtre);
							if (token_filtre2 != NULL) {
								strcpy(liste_cameras[compteur_device].options[compteur_resolution].framerate,token_filtre);
								}
							else {
								strcpy(liste_cameras[compteur_device].options[compteur_resolution].framerate,token_caps);
								}
							compteur_resolution++;
							
							}
						compteur_strtok_caps += 1;
						token_caps = strtok_r(NULL,",",&context_caps);
						}
								}}	
				}
			if ((mode_parseur == 5) && (token[4]) == 'p'){
				//trouve la fin des resolutions
				buff_compteur_path = compteur_fgets;
				token_precedent = token;
				mode_parseur = 6;
				}
			if ((mode_parseur == 6) && (token_precedent[2] == 'd') && (token_precedent[3] == 'e')&& (token_precedent[4] == 'v') && (token_precedent[5] == 'i') && (token_precedent[6] == 'c') && (token_precedent[7] == 'e') && (token_precedent[8] == '.') && (token_precedent[9] == 'p') && (token_precedent[10] == 'a') && (token_precedent[11] == 't') && (token_precedent[12] == 'h')){
				//trouve l'endroit ou le path est marqué 
				buff_compteur_path = compteur_fgets;
				mode_parseur = 7;
				}
				
			if ((buff_compteur_path + 1 == compteur_fgets) && (mode_parseur == 7)){
				//recuprère le path
				buff_compteur_path = -1;
				strcpy(buff_caracteristique,token);
				memmove(buff_caracteristique, buff_caracteristique+2, strlen(buff_caracteristique));
				strcpy(liste_cameras[compteur_device].path,buff_caracteristique);
				mode_parseur = 0;
				}
			
			token = strtok_r(NULL,"",&context_parse);
			}
		
		}
	
	int compteur_camera;
	for(compteur_camera = 0; compteur_camera < compteur_device + 1; compteur_camera++){
		strcpy(liste_cameras[compteur_camera].nom ,tableau_nom[compteur_camera]);}
	
	//uncomment here to test things.
	
	int i,j;
	
	for (i = 0;i < compteur_device+1 ; i++){
		
		printf("i = %i\n",i);
		printf("nom: %s\n",liste_cameras[i].nom);
		printf("path :%s\n",liste_cameras[i].path);
		for (j = 0;j < 100; j++) {
			printf("j = %i\n",j);
			printf("hauteur: %s\n",liste_cameras[i].options[j].hauteur);
			printf("largeur: %s\n",liste_cameras[i].options[j].largeur);
			printf("framerate: %s\n",liste_cameras[i].options[j].framerate);
			}
	}

	fclose(file_parsed);

	}
