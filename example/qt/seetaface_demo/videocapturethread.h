#ifndef VIDEOCAPTURETHREAD_H
#define VIDEOCAPTURETHREAD_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "seeta/FaceLandmarker.h"
#include "seeta/FaceDetector.h"
#include "seeta/FaceAntiSpoofing.h"
#include "seeta/Common/Struct.h"
#include "seeta/CTrackingFaceInfo.h"
#include "seeta/FaceTracker.h"
#include "seeta/FaceRecognizer.h"
#include "seeta/QualityAssessor.h"
#include "seeta/QualityOfPoseEx.h"
#include "seeta/QualityOfLBN.h"


#include <QThread>
#include <QImage>
#include <QMutex>
#include <chrono>

#include <map>

typedef struct RecognizeType
{
    int type;            //0: open camera, 1:open video file, 2:open image file
    QString filename;    //when type is 1 or 2, video file name or image file name
    QString title;       //windows title
}RecognizeType;

typedef struct DataInfo
{
    int id;
    int x;
    int y;
    int width;
    int height;
    QString name;
    QString image_path;
    float features[1024];
}DataInfo;


typedef struct Config_Paramter
{
    int MinFaceSize;
    float Fd_Threshold;
    int  VideoWidth;
    int  VideoHeight;

    float YawLowThreshold;
    float YawHighThreshold;
    float PitchLowThreshold;
    float PitchHighThreshold;

    float AntiSpoofClarity;
    float AntiSpoofReality;

    float Fr_Threshold;
    QString Fr_ModelPath;
} Config_Paramter;


typedef struct Fr_DataInfo
{
    int pid;
    int state;
}Fr_DataInfo;

class VideoCaptureThread;

class WorkThread : public QThread
{
    Q_OBJECT
public:
    WorkThread(VideoCaptureThread * main);
    ~WorkThread();

protected:
    void run();


signals:
    void sigRecognize(int, const QString &, const QString &, float, const QImage &, const QRect &);

private:

    int recognize(const SeetaTrackingFaceInfo & faceinfo);//, std::vector<unsigned int> & datas);

public:

    VideoCaptureThread * m_mainthread;
    std::vector<unsigned int> m_lastpids;
    std::vector<Fr_DataInfo> m_lasterrorpids;
};


class ResetModelThread : public QThread
{
    Q_OBJECT
public:
    ResetModelThread( const QString &imagepath, const QString & tmpimagepath);
    ~ResetModelThread();

    void start(std::map<int, DataInfo *> *datalst, const QString & table,  seeta::FaceRecognizer * fr);
protected:
    void run();


signals:
    //void sigResetModelUpdateUI(std::vector<DataInfo *> *);
    void sigResetModelEnd(int);
    void sigprogress(float);

public:

     seeta::FaceRecognizer * m_fr;
    //VideoCaptureThread * m_mainthread;

    std::map<int, DataInfo *>  * m_datalst;

    QString             m_table;
    QString             m_image_path;
    QString             m_image_tmp_path;

    bool                m_exited;
};


class InputFilesThread : public QThread
{
    Q_OBJECT
public:
    InputFilesThread(VideoCaptureThread * main, const QString &imagepath, const QString & tmpimagepath);
    ~InputFilesThread();

    void start(const QStringList * files, unsigned int id, const QString & table);
protected:
    void run();


signals:
    void sigInputFilesUpdateUI(std::vector<DataInfo *> *);
    void sigInputFilesEnd();
    void sigprogress(float);

public:

    VideoCaptureThread * m_mainthread;

    const QStringList * m_files;
    unsigned int        m_id;
    QString             m_table;
    QString             m_image_path;
    QString             m_image_tmp_path;

    bool                m_exited;
};


class VideoCaptureThread : public QThread
{
    Q_OBJECT
public:
    VideoCaptureThread(std::map<int, DataInfo *> * datalst, int videowidth, int videoheight);
    ~VideoCaptureThread();
    //void setMinFaceSize(int size);

    void setparamter();
    int checkimage(const QString & image, const QString & crop, float * features, SeetaRect &rect);

    void start(const RecognizeType &type);

    seeta::FaceRecognizer * CreateFaceRecognizer(const QString & modelfile);
    void set_fr(seeta::FaceRecognizer * fr);
protected:
    void run();


signals:
    void sigUpdateUI(const QImage & image);
    void sigEnd(int);

private:

    cv::VideoCapture * m_capture;

public:
    seeta::FaceDetector * m_fd;
    seeta::FaceLandmarker * m_pd;
    seeta::FaceLandmarker * m_pd68;
    seeta::FaceAntiSpoofing * m_spoof;
    seeta::FaceRecognizer   * m_fr;
    seeta::FaceTracker      * m_tracker;
    seeta::QualityAssessor  * m_qa;
    seeta::QualityOfLBN     * m_lbn;
    seeta::QualityOfPoseEx  * m_poseex;

public:
    bool m_isrun;
    bool m_exited;



    std::map<int, DataInfo *> *m_datalst;

    bool            m_readimage;
    SeetaImageData *m_mainImage;
    cv::Mat         m_mainmat;

    std::vector<SeetaTrackingFaceInfo> m_mainfaceinfos;

    WorkThread * m_workthread;
    QMutex m_mutex;

    RecognizeType m_type;
};

#endif // VIDEOCAPTURETHREAD_H
