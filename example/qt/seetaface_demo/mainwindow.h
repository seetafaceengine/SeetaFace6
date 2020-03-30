#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include<QTimer>

/*
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "seeta/FaceLandmarker.h"
#include "seeta/FaceDetector.h"
#include "seeta/FaceAntiSpoofing.h"
#include "seeta/Common/Struct.h"
*/

#include "videocapturethread.h"

#include "qsqldatabase.h"
#include "qsqltablemodel.h"
#include "qstandarditemmodel.h"

#include <map>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void getdatas();
    void cleardata();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    //void on_pushButton_clicked();

    void editrecord();
    void deleterecord();
    void onupdateui(const QImage & image);
    void onrecognize(int pid, const QString & name, const QString & imagepath, float score, const QImage &image, const QRect &rc);

    void onvideothreadend(int value);
    void on_dbsavebtn_clicked();

    void on_previewrunbtn_clicked();

    void on_previewstopbtn_clicked();

    void on_settingsavebtn_clicked();

    void on_rotatebtn_clicked();



    void showfaceinfo();

    void on_tabWidget_currentChanged(int index);

    void on_addimagebtn_clicked();

    void on_menufacedbbtn_clicked();

    //void on_pushButton_8_clicked();

    void on_menusettingbtn_clicked();

    void on_previewclearbtn_clicked();

    void on_menuopenvideofile_clicked();

    void on_menuopenpicturefile_clicked();

    void on_menuopencamera_clicked();

    void on_addfilesbtn_clicked();

    void oninputfilesupdateui(std::vector<DataInfo *> *);

    void on_settingselectmodelbtn_clicked();

private:
    Ui::MainWindow *ui;

    /*
    QTimer *m_timer;
    cv::VideoCapture * m_capture;

    seeta::FaceDetector * m_fd;
    seeta::FaceLandmarker * m_pd;
    seeta::FaceAntiSpoofing * m_spoof;
    */

    VideoCaptureThread * m_videothread;

    QSqlDatabase  m_database;

    // QSqlTableModel * m_model;
    QString          m_table;
    QString          m_config_table;
    QStandardItemModel * m_model;

    QPixmap          m_default_image;

    //QString          m_currentimagefile;
    QString          m_image_path;
    QString          m_image_tmp_path;
    //QString          m_model_path;

    std::map<int, DataInfo *> m_datalst;

    int              m_currenttab;

    QStandardItemModel * m_videomodel;

    RecognizeType        m_type;

    InputFilesThread     *m_inputfilesthread;
    ResetModelThread     *m_resetmodelthread;


};

#endif // MAINWINDOW_H
