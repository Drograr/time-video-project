/********************************************************************************
** Form generated from reading UI file 'tvmainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TVMAINWINDOW_H
#define UI_TVMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TVMainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QGroupBox *videoBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_7;
    QComboBox *CameracomboBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QComboBox *videoComboBox;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_4;
    QSpinBox *videoResWidthSpinBox;
    QLabel *label_5;
    QSpinBox *videoResHeightSpinBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QSpinBox *videoQuantizerSpinBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QSpinBox *videoSpeedSpinBox;
    QGroupBox *soundBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QSpinBox *audioQualitySpinBox;
    QWidget *videoWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *startButton;
    QPushButton *stopButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *TVMainWindow)
    {
        if (TVMainWindow->objectName().isEmpty())
            TVMainWindow->setObjectName(QStringLiteral("TVMainWindow"));
        TVMainWindow->resize(545, 647);
        centralWidget = new QWidget(TVMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        videoBox = new QGroupBox(centralWidget);
        videoBox->setObjectName(QStringLiteral("videoBox"));
        verticalLayout_2 = new QVBoxLayout(videoBox);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setSizeConstraint(QLayout::SetMinAndMaxSize);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        label_7 = new QLabel(videoBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        horizontalLayout_7->addWidget(label_7);

        CameracomboBox = new QComboBox(videoBox);
        CameracomboBox->setObjectName(QStringLiteral("CameracomboBox"));

        horizontalLayout_7->addWidget(CameracomboBox);


        verticalLayout_2->addLayout(horizontalLayout_7);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_6 = new QLabel(videoBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        horizontalLayout_6->addWidget(label_6);

        videoComboBox = new QComboBox(videoBox);
        videoComboBox->setObjectName(QStringLiteral("videoComboBox"));

        horizontalLayout_6->addWidget(videoComboBox);


        verticalLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_4 = new QLabel(videoBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout_5->addWidget(label_4);

        videoResWidthSpinBox = new QSpinBox(videoBox);
        videoResWidthSpinBox->setObjectName(QStringLiteral("videoResWidthSpinBox"));
        videoResWidthSpinBox->setMaximumSize(QSize(100, 16777215));
        videoResWidthSpinBox->setMaximum(4096);
        videoResWidthSpinBox->setSingleStep(10);
        videoResWidthSpinBox->setValue(640);

        horizontalLayout_5->addWidget(videoResWidthSpinBox);

        label_5 = new QLabel(videoBox);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setMaximumSize(QSize(10, 16777215));

        horizontalLayout_5->addWidget(label_5);

        videoResHeightSpinBox = new QSpinBox(videoBox);
        videoResHeightSpinBox->setObjectName(QStringLiteral("videoResHeightSpinBox"));
        videoResHeightSpinBox->setMaximumSize(QSize(100, 16777215));
        videoResHeightSpinBox->setMaximum(2160);
        videoResHeightSpinBox->setSingleStep(10);
        videoResHeightSpinBox->setValue(480);

        horizontalLayout_5->addWidget(videoResHeightSpinBox);


        verticalLayout_2->addLayout(horizontalLayout_5);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label = new QLabel(videoBox);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout_2->addWidget(label);

        videoQuantizerSpinBox = new QSpinBox(videoBox);
        videoQuantizerSpinBox->setObjectName(QStringLiteral("videoQuantizerSpinBox"));
        videoQuantizerSpinBox->setMinimumSize(QSize(50, 0));
        videoQuantizerSpinBox->setMaximumSize(QSize(50, 16777215));
        videoQuantizerSpinBox->setMaximum(50);
        videoQuantizerSpinBox->setValue(18);

        horizontalLayout_2->addWidget(videoQuantizerSpinBox);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_2 = new QLabel(videoBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_3->addWidget(label_2);

        videoSpeedSpinBox = new QSpinBox(videoBox);
        videoSpeedSpinBox->setObjectName(QStringLiteral("videoSpeedSpinBox"));
        videoSpeedSpinBox->setMinimumSize(QSize(50, 0));
        videoSpeedSpinBox->setMaximumSize(QSize(50, 16777215));
        videoSpeedSpinBox->setMaximum(10);
        videoSpeedSpinBox->setValue(1);

        horizontalLayout_3->addWidget(videoSpeedSpinBox);


        verticalLayout_2->addLayout(horizontalLayout_3);


        verticalLayout->addWidget(videoBox);

        soundBox = new QGroupBox(centralWidget);
        soundBox->setObjectName(QStringLiteral("soundBox"));
        horizontalLayout_4 = new QHBoxLayout(soundBox);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setSizeConstraint(QLayout::SetMinAndMaxSize);
        label_3 = new QLabel(soundBox);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setEnabled(true);

        horizontalLayout_4->addWidget(label_3);

        audioQualitySpinBox = new QSpinBox(soundBox);
        audioQualitySpinBox->setObjectName(QStringLiteral("audioQualitySpinBox"));
        audioQualitySpinBox->setMinimumSize(QSize(50, 0));
        audioQualitySpinBox->setMaximumSize(QSize(50, 16777215));
        audioQualitySpinBox->setMaximum(9);
        audioQualitySpinBox->setValue(2);

        horizontalLayout_4->addWidget(audioQualitySpinBox);


        verticalLayout->addWidget(soundBox);

        videoWidget = new QWidget(centralWidget);
        videoWidget->setObjectName(QStringLiteral("videoWidget"));
        videoWidget->setMinimumSize(QSize(50, 50));

        verticalLayout->addWidget(videoWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        startButton = new QPushButton(centralWidget);
        startButton->setObjectName(QStringLiteral("startButton"));

        horizontalLayout->addWidget(startButton);

        stopButton = new QPushButton(centralWidget);
        stopButton->setObjectName(QStringLiteral("stopButton"));
        stopButton->setEnabled(false);

        horizontalLayout->addWidget(stopButton);


        verticalLayout->addLayout(horizontalLayout);

        TVMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(TVMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 545, 22));
        TVMainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(TVMainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        TVMainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(TVMainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        TVMainWindow->setStatusBar(statusBar);

        retranslateUi(TVMainWindow);

        QMetaObject::connectSlotsByName(TVMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *TVMainWindow)
    {
        TVMainWindow->setWindowTitle(QApplication::translate("TVMainWindow", "TVMainWindow", Q_NULLPTR));
        videoBox->setTitle(QApplication::translate("TVMainWindow", "Video configuration (h264)", Q_NULLPTR));
        label_7->setText(QApplication::translate("TVMainWindow", "Camera", Q_NULLPTR));
        label_6->setText(QApplication::translate("TVMainWindow", "Frame rate", Q_NULLPTR));
        label_4->setText(QApplication::translate("TVMainWindow", "Resolution (width \303\227 height)", Q_NULLPTR));
        label_5->setText(QApplication::translate("TVMainWindow", "x", Q_NULLPTR));
        label->setText(QApplication::translate("TVMainWindow", "Quantizer [0-50] 0: lossless, 18: visually perfect, 21 is good", Q_NULLPTR));
        label_2->setText(QApplication::translate("TVMainWindow", "Speed [0-10] 0:none, 1-10 ultrafast to veryslow", Q_NULLPTR));
        soundBox->setTitle(QApplication::translate("TVMainWindow", "Audio configuration (FLAC)", Q_NULLPTR));
        label_3->setText(QApplication::translate("TVMainWindow", "Quality [0-9]\342\227\213 0: fast but low compr., 9: insane compression", Q_NULLPTR));
        startButton->setText(QApplication::translate("TVMainWindow", "Start recording", Q_NULLPTR));
        stopButton->setText(QApplication::translate("TVMainWindow", "Stop recording", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class TVMainWindow: public Ui_TVMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TVMAINWINDOW_H
