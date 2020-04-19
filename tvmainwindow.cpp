#include "tvmainwindow.h"
#include "ui_tvmainwindow.h"
#include <QSettings>
#include <QSize>
#include <glib/gprintf.h>
#include <gst/gst.h>
#ifdef Q_OS_LINUX
	#include <unistd.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <string.h>
	#include <stdlib.h>
	#include <sys/wait.h>
#endif



TVMainWindow::TVMainWindow(QWidget *parent, char* filename) :
    QMainWindow(parent),
    ui(new Ui::TVMainWindow)
{	
    ui->setupUi(this);
    recorder = new TVGSRecorder(filename);

    //Connect handler to manage recorder started and finished events
    connect(recorder, SIGNAL(started()), this, SLOT(cb_recorder_started()));
    connect(recorder, SIGNAL(finished()), this, SLOT(cb_recorder_finished()));

  //add items into the combobox

	camera_caps();
	sleep(1);
	QStringList Camera;
	int c;

		for (c = 0;c < nbr_cameras ; c++){

	     Camera << liste_cameras[c].nom;

			}

			ui->CameracomboBox->addItems(Camera);

			QStringList video;
			int v;

				for (v = 0;v < liste_cameras[0].nbr_resolution ; v++){


					

					QString fullresolution ( "hauteur :");
					fullresolution.append( liste_cameras[0].options[v].hauteur);
					fullresolution.append( ", largeur :");
					fullresolution.append(liste_cameras[0].options[v].largeur);
					fullresolution.append( ", framerate :");
					fullresolution.append(liste_cameras[0].options[v].framerate);

			     video << fullresolution;
					}
	       ui->videoComboBox->addItems(video);





	 connect(	ui->CameracomboBox,SIGNAL(currentTextChanged(const QString &)),this,SLOT(UpdateCombo()));




    //Load last user settings
    QSettings settings(QApplication::applicationDirPath() + "/TimedVideo.ini", QSettings::IniFormat);
    int videoQuantizerSpinBox= settings.value("video/quantizer",18).toInt();
    int videoSpeedSpinBox = settings.value("video/speed", 1).toInt();
    int audioQualitySpinBox = settings.value("audio/quality", 2).toInt();
    //ui->videoComboBox->setValue(settings.value("video/framerate", 30).toInt());
    ui->videoResWidthSpinBox->setValue(settings.value("video/width", 640).toInt());
    ui->videoResHeightSpinBox->setValue(settings.value("video/height", 480).toInt());
    ui->videoQuantizerSpinBox->setValue(videoQuantizerSpinBox);
    ui->videoSpeedSpinBox->setValue(videoSpeedSpinBox);
    ui->audioQualitySpinBox->setValue(audioQualitySpinBox);

}

TVMainWindow::~TVMainWindow()
{
    //Save User Settings
    QSettings settings(QApplication::applicationDirPath() + "/TimedVideo.ini", QSettings::IniFormat);
    settings.setValue("video/framerate",ui->videoComboBox->currentText());
    settings.setValue("video/width",ui->videoResWidthSpinBox->value());
    settings.setValue("video/height",ui->videoResHeightSpinBox->value());
    settings.setValue("video/quantizer",ui->videoQuantizerSpinBox->value());
    settings.setValue("video/speed",ui->videoSpeedSpinBox->value());
    settings.setValue("audio/quality",ui->audioQualitySpinBox->value());

    //Clean up the class member variables
    delete ui;
    delete recorder;
}

void TVMainWindow::on_startButton_clicked()
{
    //Initialize the recording player if needed
    QSize videoSize(ui->videoResWidthSpinBox->value(), ui->videoResHeightSpinBox->value());
    if(recorder->init_pipeline(ui->videoComboBox->currentText(), videoSize, ui->videoQuantizerSpinBox->value(), ui->videoSpeedSpinBox->value(), ui->audioQualitySpinBox->value()) == false)
        return;

    //Associate recorder video output the video widget (should be done each time, why ?)
    recorder->setDisplay(ui->videoWidget);

    //Start playing
    recorder->start();
}

void TVMainWindow::on_stopButton_clicked()
{
    recorder->stop();
}

void TVMainWindow::cb_recorder_started()
{
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
}

void TVMainWindow::cb_recorder_finished()
{
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void TVMainWindow::closeEvent (QCloseEvent *event)
{
    recorder->stop();
    recorder->wait();
}


void TVMainWindow::UpdateCombo(){
ui->videoComboBox->clear();
QStringList video;
int v;

	for (v = 0;v < liste_cameras[ui->CameracomboBox->currentIndex()].nbr_resolution ; v++){


		QString fullresolution ( "hauteur :");
		fullresolution.append( liste_cameras[ui->CameracomboBox->currentIndex()].options[v].hauteur);
		fullresolution.append( ", largeur :");
		fullresolution.append(liste_cameras[ui->CameracomboBox->currentIndex()].options[v].largeur);
		fullresolution.append( ", framerate :");
		fullresolution.append(liste_cameras[ui->CameracomboBox->currentIndex()].options[v].framerate);

		 video << fullresolution;
		}
	 ui->videoComboBox->addItems(video);
}

static gboolean my_bus_func(GstBus * bus, GstMessage * message, gpointer user_data)
   {
      GstDevice *device;
      gchar *name;

      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_DEVICE_ADDED:
          gst_message_parse_device_added (message, &device);
          name = gst_device_get_display_name (device);
          g_print("Device added: %s\n", name);
          g_free (name);
          gst_object_unref (device);
          break;
        case GST_MESSAGE_DEVICE_REMOVED:
          gst_message_parse_device_removed (message, &device);
          name = gst_device_get_display_name (device);
          g_print("Device removed: %s\n", name);
          g_free (name);
          gst_object_unref (device);
          break;
        default:
          break;
      }

      return G_SOURCE_CONTINUE;
   }

   GstDeviceMonitor *setup_raw_video_source_device_monitor (void) {
      GstDeviceMonitor *monitor;
      GstBus *bus;
      GstCaps *caps;

      monitor = gst_device_monitor_new ();

      bus = gst_device_monitor_get_bus (monitor);
      gst_bus_add_watch (bus, my_bus_func, NULL);
      gst_object_unref (bus);

      caps = gst_caps_new_empty_simple ("video/x-raw");
      gst_device_monitor_add_filter (monitor, "Video/Source", caps);
      gst_caps_unref (caps);

      gst_device_monitor_start (monitor);

      return monitor;
   }

void TVMainWindow::camera_caps()
{	
	unsigned int compteur_inspecteur1;
	GstDeviceMonitor *monitor; 
	nbr_cameras = 0;

	monitor = setup_raw_video_source_device_monitor();
	GList *list_gstdevice = gst_device_monitor_get_devices(monitor);
	guint taille_liste_device_uint = g_list_length(list_gstdevice);
	
	
	while(list_gstdevice != NULL){
		gchar *nom_camera_monitorer = gst_device_get_display_name((GstDevice*)list_gstdevice->data);
		//printf("%s\n",(char *)nom_camera_monitorer);
		strcpy(liste_cameras[nbr_cameras].nom,(char *)nom_camera_monitorer);
		nbr_cameras ++;
		GstStructure *test_property = gst_device_get_properties((GstDevice*)list_gstdevice->data);
		gchar *test_property_char = gst_structure_to_string(test_property);
		printf("%s\n",(char *)test_property_char);
		
		GstCaps *Caps_current = gst_device_get_caps((GstDevice*)list_gstdevice->data);
		guint size_caps_current = gst_caps_get_size(Caps_current);
		
		for(compteur_inspecteur1 = 0; compteur_inspecteur1 < size_caps_current;compteur_inspecteur1 ++){
			GstCaps *Bout_de_Caps = gst_caps_copy_nth(Caps_current,compteur_inspecteur1);
			//g_printf("%s\n",gst_caps_to_string(Bout_de_Caps));
		}
		list_gstdevice = list_gstdevice->next;
		
	}
}

