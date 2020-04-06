#include "tvmainwindow.h"
#include "ui_tvmainwindow.h"
#include <QSettings>
#include <QSize>
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
   ui->videoComboBox->addItem( QString("30") );
   ui->videoComboBox->addItem( QString("25") );
   ui->videoComboBox->addItem( QString("20") );
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
	camera_caps();
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
#ifdef Q_OS_LINUX
void TVMainWindow::camera_caps()
{
	
	int compteur_fgets = 0,mode_parseur = 0,buff_compteur_fgets = 0;
	int compteur_strtok_caps = 0;
	char *token;
	char *token_caps;
	char string_container[100000];
	char nom_device[1000]; 
	char type_device_video[1000];
	char buff_caracteristique[1000];
	int flag_mode4 = 0;
	char *context_parse;
	char *context_caps;
	int buff_compteur_path = -1;
	int compteur_resolution_buffeur;
	char *token_precedent = NULL;
	
	
	
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
	
	
	FILE *file_parsed = fopen("./test.txt","r");
	
	
	while (fgets(string_container, 100000, file_parsed) != NULL){
		
		compteur_fgets += 1;
		token = strtok_r(string_container," ",&context_parse);
		while (token != NULL){
			printf("mode parseur = %i\n",mode_parseur);
			printf("%s\n",token);
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
					liste_cameras[compteur_device].nbr_resolution = -1;
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
							
							}
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
							liste_cameras[compteur_device].nbr_resolution += 1;
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
	
	nbr_cameras = compteur_device + 1;
	int compteur_camera;
	for(compteur_camera = 0; compteur_camera < compteur_device + 1; compteur_camera++){
		strcpy(liste_cameras[compteur_camera].nom ,tableau_nom[compteur_camera]);}
	
	//uncomment here to test things.
	
	int i,j;
	printf("Nombre camera: %i\n",nbr_cameras);
	for (i = 0;i < compteur_device+1 ; i++){
		
		printf("i = %i\n",i);
		printf("nom: %s\n",liste_cameras[i].nom);
		printf("path :%s\n",liste_cameras[i].path);
		printf("nbr_res:%i\n",liste_cameras[i].nbr_resolution); 
		for (j = 0;j < 100; j++) {
			printf("j = %i\n",j);
			printf("hauteur: %s\n",liste_cameras[i].options[j].hauteur);
			printf("largeur: %s\n",liste_cameras[i].options[j].largeur);
			printf("framerate: %s\n",liste_cameras[i].options[j].framerate);
			}
	}

	fclose(file_parsed);

	}
#endif
