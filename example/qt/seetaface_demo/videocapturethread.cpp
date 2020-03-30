#include "videocapturethread.h"



#include "seeta/QualityOfPoseEx.h"
#include "seeta/Struct.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include "QDebug"


using namespace std::chrono;

extern const QString gcrop_prefix;
extern Config_Paramter gparamters;
extern std::string gmodelpath;// = "/wqy/seeta_sdk/SF3/libs/SF3.0_v1/models/";

void clone_image( const SeetaImageData &src, SeetaImageData &dst)
{
    if(src.width != dst.width  || src.height != dst.height || src.channels != dst.channels)
    {
        if(dst.data)
        {
            delete [] dst.data;
            dst.data = nullptr;
        }
        dst.width = src.width;
        dst.height = src.height;
        dst.channels = src.channels;
        dst.data = new unsigned char[src.width * src.height * src.channels];
    }

    memcpy(dst.data, src.data, src.width * src.height * src.channels);
}

//////////////////////////////
WorkThread::WorkThread(VideoCaptureThread * main)
{
    m_mainthread = main;

}

WorkThread::~WorkThread()
{
    qDebug() << "WorkThread exited";
}

int WorkThread::recognize(const SeetaTrackingFaceInfo & faceinfo)//, std::vector<unsigned int> & datas)
{
    auto points = m_mainthread->m_pd->mark(*m_mainthread->m_mainImage, faceinfo.pos);

    m_mainthread->m_qa->feed(*(m_mainthread->m_mainImage), faceinfo.pos, points.data(), 5);
    auto result1 = m_mainthread->m_qa->query(seeta::BRIGHTNESS);
    auto result2 = m_mainthread->m_qa->query(seeta::RESOLUTION);
    auto result3 = m_mainthread->m_qa->query(seeta::CLARITY);
    auto result4 = m_mainthread->m_qa->query(seeta::INTEGRITY);
    auto result = m_mainthread->m_qa->query(seeta::POSE_EX);

    qDebug() << "PID:" << faceinfo.PID;
    if(result.level == 0 || result1.level == 0 || result2.level == 0 || result3.level == 0 || result4.level == 0 )
    {
        qDebug() << "Quality check failed!";
        return -1;
    }

    auto status = m_mainthread->m_spoof->Predict( *m_mainthread->m_mainImage, faceinfo.pos, points.data() );

    if( status != seeta::FaceAntiSpoofing::REAL)
    {
        qDebug() << "antispoofing check failed!";
        return -2;
    }
    seeta::ImageData cropface = m_mainthread->m_fr->CropFaceV2(*m_mainthread->m_mainImage,  points.data() );
    float features[1024];
    memset(features, 0, 1024 * sizeof(float));
    m_mainthread->m_fr->ExtractCroppedFace(cropface, features);
    std::map<int, DataInfo *>::iterator iter = m_mainthread->m_datalst->begin();
    //std::vector<unsigned int> datas;

    for(; iter != m_mainthread->m_datalst->end(); ++iter)
    {
        if(m_mainthread->m_exited)
        {
            return -3;
        }
        float score = m_mainthread->m_fr->CalculateSimilarity(features, iter->second->features);
        qDebug() << "PID:" << faceinfo.PID << ", score:" << score;
        if(score >= gparamters.Fr_Threshold)
        {
            //datas.push_back(faceinfo.PID);
            //m_lastpids.push_back(faceinfo.PID);

            int x = faceinfo.pos.x - faceinfo.pos.width / 2;
            if((x) < 0)
                x = 0;
            int y = faceinfo.pos.y - faceinfo.pos.height / 2;
            if(y  < 0)
                y = 0;

            int x2 = faceinfo.pos.x + faceinfo.pos.width * 1.5;
            if(x2 >= m_mainthread->m_mainImage->width)
            {
                x2 = m_mainthread->m_mainImage->width -1;
            }

            int y2 = faceinfo.pos.y + faceinfo.pos.height * 1.5;
            if(y2 >= m_mainthread->m_mainImage->height)
            {
                y2 = m_mainthread->m_mainImage->height -1;
            }

            //qDebug() << "----x:" << faceinfo.pos.x  << ",y:" << faceinfo.pos.y << ",w:" << faceinfo.pos.width << ",h:" << faceinfo.pos.height;
            cv::Rect rect(x, y, x2-x, y2 - y);
            //qDebug() << "x:" << x << ",y:" << y << ",w:" << x2-x << ",h:" << y2-y;
            //cv::Rect rect(faceinfo.pos.x, faceinfo.pos.y, faceinfo.pos.width, faceinfo.pos.height);

            cv::Mat mat = m_mainthread->m_mainmat(rect).clone();
            //cv::imwrite("/tmp/ddd.png",mat);
            //qDebug() << "----mat---";
            QImage image((const unsigned char *)mat.data, mat.cols,mat.rows,mat.step, QImage::Format_RGB888);
            //image.save("/tmp/wwww.png");
            //qDebug() << "PID:" << faceinfo.PID << ", score:" << score;
            QRect rc(iter->second->x, iter->second->y, iter->second->width, iter->second->height);

            emit sigRecognize(faceinfo.PID, iter->second->name, iter->second->image_path, score, image, rc);
            return 0;
        }
    }

    //m_lastpids.clear();
    //m_lastpids.resize(datas.size());
    //std::copy(datas.begin(), datas.end(), m_lastpids.begin());

    return -3;
}

void WorkThread::run()
{
   //m_begin = system_clock::now();
   m_lastpids.clear();
   m_lasterrorpids.clear();
   bool bfind = false;
   std::vector<int> datas;
   std::vector<Fr_DataInfo> errordatas;
   int nret = 0;
   while(!m_mainthread->m_exited)
   {
       if(!m_mainthread->m_readimage)
       {
           QThread::msleep(1);
           continue;
       }

       auto end = system_clock::now();
       //auto duration = duration_cast<seconds>(end - m_begin);
       //int spent = duration.count();
       //if(spent > 10)
       //    m_lastpids.clear();

       datas.clear();
       errordatas.clear();
       for(int i=0; i<m_mainthread->m_mainfaceinfos.size(); i++)
       {
           if(m_mainthread->m_exited)
           {
               return;
           }

           bfind = false;
           for(int k=0; k<m_lastpids.size(); k++)
           {
               if(m_mainthread->m_mainfaceinfos[i].PID == m_lastpids[k])
               {
                   datas.push_back(m_lastpids[k]);
                   bfind = true;
                   break;
               }
           }
           if(!bfind)
           {
               SeetaTrackingFaceInfo & faceinfo = m_mainthread->m_mainfaceinfos[i];
               nret = recognize(faceinfo);//(m_mainthread->m_mainfaceinfos[i]);//, datas);
               if(nret < 0)
               {
                   Fr_DataInfo info;
                   info.pid = faceinfo.PID;
                   info.state = nret;
                   errordatas.push_back(info);
                   bool bsend = true;
                   for(int k=0; k<m_lasterrorpids.size(); k++)
                   {
                       if(info.pid == m_lasterrorpids[k].pid)
                       {
                           if(info.state == m_lasterrorpids[k].state)
                           {
                               bsend = false;
                           }

                           break;
                       }
                   }

                   if(bsend)
                   {
                       int x = faceinfo.pos.x - faceinfo.pos.width / 2;
                       if((x) < 0)
                           x = 0;
                       int y = faceinfo.pos.y - faceinfo.pos.height / 2;
                       if(y  < 0)
                           y = 0;

                       int x2 = faceinfo.pos.x + faceinfo.pos.width * 1.5;
                       if(x2 >= m_mainthread->m_mainImage->width)
                       {
                           x2 = m_mainthread->m_mainImage->width -1;
                       }

                       int y2 = faceinfo.pos.y + faceinfo.pos.height * 1.5;
                       if(y2 >= m_mainthread->m_mainImage->height)
                       {
                           y2 = m_mainthread->m_mainImage->height -1;
                       }

                       //qDebug() << "----x:" << faceinfo.pos.x  << ",y:" << faceinfo.pos.y << ",w:" << faceinfo.pos.width << ",h:" << faceinfo.pos.height;
                       cv::Rect rect(x, y, x2-x, y2 - y);
                       //qDebug() << "x:" << x << ",y:" << y << ",w:" << x2-x << ",h:" << y2-y;
                       //cv::Rect rect(faceinfo.pos.x, faceinfo.pos.y, faceinfo.pos.width, faceinfo.pos.height);

                       cv::Mat mat = m_mainthread->m_mainmat(rect).clone();
                       //cv::imwrite("/tmp/ddd.png",mat);
                       //qDebug() << "----mat---";
                       QImage image((const unsigned char *)mat.data, mat.cols,mat.rows,mat.step, QImage::Format_RGB888);

                       QString str;
                       if(info.state == -1)
                       {
                           str = "QA ERROR";
                       }else if(info.state == -2)
                       {
                           str = "SPOOFING";
                       }else if(info.state == -3)
                       {
                           str = "MISS";
                       }
                       emit sigRecognize(info.pid, "", str, 0.0, image, QRect(0,0,0,0));
                   }
               }else
               {
                   datas.push_back(m_mainthread->m_mainfaceinfos[i].PID);
               }
           }

       }

       m_lastpids.clear();
       m_lastpids.resize(datas.size());
       std::copy(datas.begin(), datas.end(), m_lastpids.begin());

       m_lasterrorpids.clear();
       m_lasterrorpids.resize(errordatas.size());
       std::copy(errordatas.begin(), errordatas.end(), m_lasterrorpids.begin());

       auto end2 = system_clock::now();
       auto duration2= duration_cast<milliseconds>(end2 - end);
       int spent2 = duration2.count();
       //qDebug() << "----spent:" << spent2;
       m_mainthread->m_mutex.lock();
       m_mainthread->m_readimage = false;
       m_mainthread->m_mutex.unlock();

   }

}


/////////////////////////////////
ResetModelThread::ResetModelThread(const QString &imagepath, const QString & tmpimagepath)
{
    //m_mainthread = main;
    m_image_path = imagepath;
    m_image_tmp_path = tmpimagepath;
    m_exited = false;
}

ResetModelThread::~ResetModelThread()
{
    qDebug() << "ResetModelThread exited";
}

void ResetModelThread::start(std::map<int, DataInfo *> *datalst, const QString & table,  seeta::FaceRecognizer * fr)
{
    m_table = table;
    m_datalst = datalst;
    m_fr = fr;
    m_exited = false;

    QThread::start();
}

typedef struct DataInfoTmp
{
    int id;
    float features[ 1024];
}DataInfoTmp;

void ResetModelThread::run()
{
     int num = m_datalst->size();
     QString fileName;

     float lastvalue = 0.0;
     float value = 0.0;


     //////////////////////////////////

     QSqlQuery query;
     query.exec("drop table " + m_table + "_tmp");
     if(!query.exec("create table " +  m_table + "_tmp (id int primary key, name varchar(64), image_path varchar(256), feature_data blob)"))
     {
         qDebug() << "failed to create table:" + m_table + "_tmp"<< query.lastError();
         emit sigResetModelEnd(-1);
         return;
     }


     ////////////////////////////////
     float features[1024];

     std::vector<DataInfoTmp *> vecs;

     std::map<int, DataInfo *>::iterator iter = m_datalst->begin();
     //std::vector<unsigned int> datas;
     int i=0;

     for(; iter != m_datalst->end(); ++iter,i++)
     {
         if(m_exited)
         {
            break;
         }

         value = (i + 1) / num;
         value = value * 90;
         if(value - lastvalue >= 1.0)
         {
             emit sigprogress(value);
             lastvalue = value;
         }
         //QString str = QString("current progress : %1%").arg(QString::number(value, 'f',1));
         emit sigprogress(value);

         fileName = m_image_path + "crop_" + iter->second->image_path;
         cv::Mat mat = cv::imread(fileName.toStdString().c_str());
         if(mat.data == NULL)
         {
             continue;
         }

         SeetaImageData image;
         image.height = mat.rows;
         image.width = mat.cols;
         image.channels = mat.channels();
         image.data = mat.data;
         memset(features, 0, 1024 * sizeof(float));
         m_fr->ExtractCroppedFace(image, features);



         ////////////////////////////////////////////////////////
         /*
         ///
         QSqlQuery query;
         query.prepare("update  " + m_table + " set feature_data = :feature_data where id=:id");

         query.bindValue(":id", iter->second->id);

         QByteArray bytearray;
         bytearray.resize(1024 * sizeof(float));
         memcpy(bytearray.data(), features, 1024 * sizeof(float));
         query.bindValue(":feature_data", QVariant(bytearray));
         if(!query.exec())
         {
              //vecs.push_back(iter->second->id);
              qDebug() << "failed to update table:" << query.lastError();
              continue;
         }
         */
         //////////////////////////////////////////////////////
         QSqlQuery query2;
         query2.prepare("insert into  " + m_table + "_tmp (id, name, image_path, feature_data) values (:id, :name, :image_path, :feature_data)");

         query2.bindValue(":id", iter->second->id);
         query2.bindValue(":name",iter->second->name);
         query2.bindValue(":image_path", iter->second->image_path);

         QByteArray bytearray;
         bytearray.resize(1024 * sizeof(float));
         memcpy(bytearray.data(), features, 1024 * sizeof(float));

         query2.bindValue(":feature_data", QVariant(bytearray));
         if(!query2.exec())
         {
              qDebug() << "failed to update table:" << query.lastError();
              continue;
              break;
         }


         ///////////////////////////////////////////////


         DataInfoTmp * info = new DataInfoTmp;
         info->id = iter->second->id;
         memcpy(info->features, features, 1024 * sizeof(float));
         vecs.push_back(info);
         memcpy(iter->second->features, features, 1024 * sizeof(float));
     }

     if(i < m_datalst->size())
     {

         QSqlQuery deltable("drop table " + m_table + "_tmp");
         deltable.exec();
         for(int k=0; k<vecs.size(); k++)
         {
             delete vecs[k];
         }
         vecs.clear();

         qDebug() << "------ResetModelThread---22:" ;
         emit sigResetModelEnd(-2);
         return;
     }else
     {
         emit sigprogress(90.0);

         //QSqlQuery renametab("drop table " + m_table  + "; alter table " + m_table + "_tmp rename to " + m_table + ";");
         QSqlQuery renamequery;
         renamequery.exec("drop table " + m_table);
         renamequery.exec("alter table " + m_table + "_tmp rename to " + m_table);
         /*
         if(!renametab.exec())
         {
             qDebug() << "------ResetModelThread---33:" << renametab.lastError();
             for(int k=0; k<vecs.size(); k++)
             {
                 delete vecs[k];
             }
             vecs.clear();
             emit sigResetModelEnd(-3);
             return;
         }
         */
         emit sigprogress(95.0);
         for(int k=0; k<vecs.size(); k++)
         {
             iter = m_datalst->find(vecs[k]->id);
             if(iter != m_datalst->end())
             {
                 memcpy(iter->second->features, vecs[k]->features, 1024 * sizeof(float));
                 delete vecs[k];
             }
         }
         vecs.clear();

     }
     emit sigprogress(100.0);
     qDebug() << "------ResetModelThread---ok:";
     emit sigResetModelEnd(0);
}
///


/////////////////////////////////
InputFilesThread::InputFilesThread(VideoCaptureThread * main, const QString &imagepath, const QString & tmpimagepath)
{
    m_mainthread = main;
    m_image_path = imagepath;
    m_image_tmp_path = tmpimagepath;
    m_exited = false;
}

InputFilesThread::~InputFilesThread()
{
    qDebug() << "InputFilesThread exited";
}

void InputFilesThread::start(const QStringList * files, unsigned int id, const QString & table)
{
    m_table = table;
    m_files = files;
    m_id = id;
    m_exited = false;
    QThread::start();
}

void InputFilesThread::run()
{
     int num = m_files->size();
     float features[1024];
     QString strerror;
     int nret;
     QString fileName;
     int index;

     float lastvalue = 0.0;
     float value = 0.0;
     SeetaRect rect;
     std::vector<DataInfo *> datalst;

     for(int i=0; i<m_files->size(); i++)
     {
         if(m_exited)
             break;
         value = (i + 1) / num;
         value = value * 100 * 0.8;
         if(value - lastvalue >= 1.0)
         {
             emit sigprogress(value);
             lastvalue = value;
         }
         QString str = QString("current progress : %1%").arg(QString::number(value, 'f',1));
         emit sigprogress(value);

         fileName = m_files->at(i);

         QImage image(fileName);
         if(image.isNull())
             continue;

         QFile file(fileName);
         QFileInfo fileinfo(fileName);

         //////////////////////////////
         QSqlQuery query;
         query.prepare("insert into  " + m_table + " (id, name, image_path, feature_data, facex,facey,facewidth,faceheight) values (:id, :name, :image_path, :feature_data,:facex,:facey,:facewidth,:faceheight)");

         index = m_id + 1;

         QString strfile = QString::number(index) + "_" + fileinfo.fileName();
         QString cropfile = m_image_path + "crop_" + strfile;

         memset(features, 0, sizeof(float) * 1024);
         nret = m_mainthread->checkimage(fileName, cropfile, features, rect);
         strerror = "";

         if(nret == -2)
         {
             strerror = "do not find face!";
         }else if(nret == -1)
         {
             strerror = fileName + " is invalid!";
         }else if(nret == 1)
         {
             strerror = "find more than one face!";
         }else if(nret == 2)
         {
             strerror = "quality check failed!";
         }

         if(!strerror.isEmpty())
         {
             //QMessageBox::critical(NULL,"critical", strerror, QMessageBox::Yes);
             continue;
         }

         QString name = fileinfo.completeBaseName();//fileName();
         int n = name.indexOf("_");

         if(n >= 1)
         {
             name = name.left(n);
         }

         query.bindValue(0, index);
         query.bindValue(1,name);
         query.bindValue(2, strfile);

         QByteArray bytearray;
         bytearray.resize(1024 * sizeof(float));
         memcpy(bytearray.data(), features, 1024 * sizeof(float));

         query.bindValue(3, QVariant(bytearray));
         query.bindValue(4, rect.x);
         query.bindValue(5, rect.y);
         query.bindValue(6, rect.width);
         query.bindValue(7, rect.height);
         if(!query.exec())
         {
              QFile::remove(cropfile);
              qDebug() << "failed to insert table:" << query.lastError();
              //QMessageBox::critical(NULL, "critical", tr("save face data to database failed!"), QMessageBox::Yes);
              continue;
         }

         file.copy(m_image_path + strfile);


         DataInfo * info = new DataInfo();
         info->id = index;
         info->name = name;
         info->image_path = strfile;
         memcpy(info->features, features, 1024 * sizeof(float));
         info->x = rect.x;
         info->y = rect.y;
         info->width = rect.width;
         info->height = rect.height;
         datalst.push_back(info);

         m_id++;
     }

     if(datalst.size() > 0)
     {
         emit sigInputFilesUpdateUI( &datalst);
     }

     emit sigprogress(100.0);

     datalst.clear();
     emit sigInputFilesEnd();
}
///

VideoCaptureThread::VideoCaptureThread(std::map<int, DataInfo *> * datalst, int videowidth, int videoheight)
{
    m_exited = false;
    //m_haveimage = false;

    m_datalst = datalst;
    //m_width = 800;
    //m_height = 600;
    qDebug() << "video width:" << videowidth << "," << videoheight;

    //std::string modelpath = "/wqy/seeta_sdk/SF3/libs/SF3.0_v1/models/";
    seeta::ModelSetting fd_model;
    fd_model.append(gmodelpath + "face_detector.csta");
    fd_model.set_device( seeta::ModelSetting::CPU );
    fd_model.set_id(0);
    m_fd = new seeta::FaceDetector(fd_model);
    m_fd->set(seeta::FaceDetector::PROPERTY_MIN_FACE_SIZE, 100);

    m_tracker = new seeta::FaceTracker(fd_model, videowidth,videoheight);
    m_tracker->SetMinFaceSize(100); //set(seeta::FaceTracker::PROPERTY_MIN_FACE_SIZE, 100);

    seeta::ModelSetting pd_model;
    pd_model.append(gmodelpath + "face_landmarker_pts5.csta");
    pd_model.set_device( seeta::ModelSetting::CPU );
    pd_model.set_id(0);
    m_pd = new seeta::FaceLandmarker(pd_model);


    seeta::ModelSetting spoof_model;
    spoof_model.append(gmodelpath + "fas_first.csta");
    spoof_model.append(gmodelpath + "fas_second.csta");
    spoof_model.set_device( seeta::ModelSetting::CPU );
    spoof_model.set_id(0);
    m_spoof = new seeta::FaceAntiSpoofing(spoof_model);
    m_spoof->SetThreshold(0.30, 0.80);

    seeta::ModelSetting fr_model;
    fr_model.append(gmodelpath + "face_recognizer.csta");
    fr_model.set_device( seeta::ModelSetting::CPU );
    fr_model.set_id(0);
    m_fr = new seeta::FaceRecognizer(fr_model);



    ///////////////////////////////
    seeta::ModelSetting setting68;
    setting68.set_id(0);
    setting68.set_device( SEETA_DEVICE_CPU );
    setting68.append(gmodelpath + "face_landmarker_pts68.csta" );
    m_pd68 = new seeta::FaceLandmarker( setting68 );

    seeta::ModelSetting posemodel;
    posemodel.set_device(SEETA_DEVICE_CPU);
    posemodel.set_id(0);
    posemodel.append(gmodelpath + "pose_estimation.csta");
    m_poseex = new seeta::QualityOfPoseEx(posemodel);
    m_poseex->set(seeta::QualityOfPoseEx::YAW_LOW_THRESHOLD, 20);
    m_poseex->set(seeta::QualityOfPoseEx::YAW_HIGH_THRESHOLD, 10);
    m_poseex->set(seeta::QualityOfPoseEx::PITCH_LOW_THRESHOLD, 20);
    m_poseex->set(seeta::QualityOfPoseEx::PITCH_HIGH_THRESHOLD, 10);

    seeta::ModelSetting lbnmodel;
    lbnmodel.set_device(SEETA_DEVICE_CPU);
    lbnmodel.set_id(0);
    lbnmodel.append(gmodelpath + "quality_lbn.csta");
    m_lbn = new seeta::QualityOfLBN(lbnmodel);
    m_lbn->set(seeta::QualityOfLBN::PROPERTY_BLUR_THRESH, 0.80);

    m_qa = new seeta::QualityAssessor();
    m_qa->add_rule(seeta::INTEGRITY);
    m_qa->add_rule(seeta::RESOLUTION);
    m_qa->add_rule(seeta::BRIGHTNESS);
    m_qa->add_rule(seeta::CLARITY);
    m_qa->add_rule(seeta::POSE_EX, m_poseex, true);

    //////////////////////


    //m_capture = new cv::VideoCapture(0);
    m_capture = NULL;//new cv::VideoCapture;
    //m_capture->set( cv::CAP_PROP_FRAME_WIDTH, videowidth );
    //m_capture->set( cv::CAP_PROP_FRAME_HEIGHT, videoheight );
    //int videow = vc.get( CV_CAP_PROP_FRAME_WIDTH );
    //int videoh = vc.get( CV_CAP_PROP_FRAME_HEIGHT );

    m_workthread = new WorkThread(this);

    m_mainImage = new SeetaImageData();
    //m_curImage = new SeetaImageData();
    m_mainImage->width = m_mainImage->height = m_mainImage->channels= 0;
    m_mainImage->data = NULL;

    //m_curImage->width = m_curImage->height = m_curImage->channels= 0;
    //m_curImage->data = NULL;
}

VideoCaptureThread::~VideoCaptureThread()
{
    m_exited = true;
    while(!isFinished())
    {
        QThread::msleep(1);
    }
    qDebug() << "VideoCaptureThread exited";
    if( m_capture)
        delete m_capture;
    delete m_fd;
    delete m_pd;
    delete m_spoof;
    delete m_tracker;
    delete m_lbn;
    delete m_qa;

    delete m_workthread;

}

void VideoCaptureThread::setparamter()
{
    /*
    qDebug() << gparamters.MinFaceSize << ", " << gparamters.Fd_Threshold;
    qDebug() << gparamters.VideoWidth << ", " << gparamters.VideoHeight;
    qDebug() << gparamters.AntiSpoofClarity << ", " << gparamters.AntiSpoofReality;
    qDebug() << gparamters.YawLowThreshold << ", " << gparamters.YawHighThreshold;
    qDebug() << gparamters.PitchLowThreshold << ", " << gparamters.PitchHighThreshold;
    */
    m_fd->set(seeta::FaceDetector::PROPERTY_MIN_FACE_SIZE, gparamters.MinFaceSize);
    m_fd->set(seeta::FaceDetector::PROPERTY_THRESHOLD, gparamters.Fd_Threshold);

    m_tracker->SetMinFaceSize(gparamters.MinFaceSize);
    m_tracker->SetThreshold(gparamters.Fd_Threshold);
    m_tracker->SetVideoSize(gparamters.VideoWidth, gparamters.VideoHeight);

    m_spoof->SetThreshold(gparamters.AntiSpoofClarity, gparamters.AntiSpoofReality);

    m_poseex->set(seeta::QualityOfPoseEx::YAW_LOW_THRESHOLD, gparamters.YawLowThreshold);
    m_poseex->set(seeta::QualityOfPoseEx::YAW_HIGH_THRESHOLD, gparamters.YawHighThreshold);
    m_poseex->set(seeta::QualityOfPoseEx::PITCH_LOW_THRESHOLD, gparamters.PitchLowThreshold);
    m_poseex->set(seeta::QualityOfPoseEx::PITCH_HIGH_THRESHOLD, gparamters.PitchHighThreshold);

}

seeta::FaceRecognizer * VideoCaptureThread::CreateFaceRecognizer(const QString & modelfile)
{

    seeta::ModelSetting fr_model;
    fr_model.append(gmodelpath + modelfile.toStdString());
    fr_model.set_device( seeta::ModelSetting::CPU );
    fr_model.set_id(0);
    seeta::FaceRecognizer * fr = new seeta::FaceRecognizer(fr_model);
    return fr;
}

void VideoCaptureThread::set_fr(seeta::FaceRecognizer * fr)
{
    if(m_fr != NULL)
    {
        delete m_fr;
    }
    m_fr = fr;
}

void VideoCaptureThread::start(const RecognizeType &type)
{
    m_type.type = type.type;
    m_type.filename = type.filename;
    QThread::start();
}

void VideoCaptureThread::run()
{
    int nret = 0;


    if(m_type.type == 0)
    {
        m_capture = new cv::VideoCapture;
        m_capture->open(m_type.type);
        m_capture->set( cv::CAP_PROP_FRAME_WIDTH, gparamters.VideoWidth );
        m_capture->set( cv::CAP_PROP_FRAME_HEIGHT, gparamters.VideoHeight );

    }else if(m_type.type == 1)
    {
        m_capture = new cv::VideoCapture;
        m_capture->open(m_type.filename.toStdString().c_str());
        m_capture->set( cv::CAP_PROP_FRAME_WIDTH, gparamters.VideoWidth );
        m_capture->set( cv::CAP_PROP_FRAME_HEIGHT, gparamters.VideoHeight );
    }

    //m_capture->open("/tmp/test.avi");
    //m_capture->open(0);
    //m_capture->set( cv::CAP_PROP_FRAME_WIDTH, gparamters.VideoWidth );
    //m_capture->set( cv::CAP_PROP_FRAME_HEIGHT, gparamters.VideoHeight );


    if((m_capture != NULL) && (!m_capture->isOpened()))
    {
        m_capture->release();
        emit sigEnd(-1);
        return;
    }

    cv::Mat mat, mat2;
    cv::Scalar color;
    color = CV_RGB( 0, 255, 0 );

    m_workthread->start();

    /*
    //mp4,h263,flv
    cv::VideoWriter outputvideo;
    cv::Size s(800,600);
    int codec = outputvideo.fourcc('M', 'P', '4', '2');
    outputvideo.open("/tmp/test.avi", codec, 50.0, s, true);
    if(!outputvideo.isOpened())
    {
        qDebug() << " write video failed";
    }
    */

    while(!m_exited)
    {
        if(m_type.type == 2)
        {
            mat = cv::imread(m_type.filename.toStdString().c_str());
            if(mat.data == NULL)
            {
                qDebug() << "VideoCapture read failed";
                m_exited = true;
                nret = -2;
                break;
            }
        }else
        {
            if(!m_capture->read(mat))
            {
                qDebug() << "VideoCapture read failed";
                m_exited = true;
                nret = -2;
                break;
            }
        }

        //(*m_capture) >> mat;

        //cv::imwrite("/tmp/www_test.png",mat);
        auto start = system_clock::now();
        if(m_type.type == 1)
        {
            cv::flip(mat, mat, 1);
        }else
        {
            cv::Size size (gparamters.VideoWidth, gparamters.VideoHeight);
            cv::resize(mat, mat2, size, 0, 0, cv::INTER_CUBIC);
            mat = mat2.clone();
        }

        if(mat.channels() == 4)
        {
            cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGR);
        }

        SeetaImageData image;
        image.height = mat.rows;
        image.width = mat.cols;
        image.channels = mat.channels();
        image.data = mat.data;

        cv::cvtColor(mat, mat2, cv::COLOR_BGR2RGB);

        auto faces = m_tracker->Track(image);
        //qDebug() << "-----track size:" << faces.size;
        if( faces.size > 0 )
        {
            m_mutex.lock();
            if(!m_readimage)
            {
                clone_image(image, *m_mainImage);
                //cv::Mat tmpmat;
                //cv::cvtColor(mat, tmpmat, cv::COLOR_BGR2RGB);
                m_mainmat = mat2.clone();//tmpmat.clone();
                m_mainfaceinfos.clear();
                for(int i=0; i<faces.size; i++)
                {
                    m_mainfaceinfos.push_back(faces.data[i]);
                }
                m_readimage = true;
            }
            m_mutex.unlock();


            for(int i=0; i<faces.size; i++)
            {
                auto &face = faces.data[i].pos;
                //std::cout << "Clarity = " << clarity << ", Reality = " << reality << std::endl;
                //auto end = system_clock::now();
                //auto duration = duration_cast<microseconds>(end - start);
                //int spent = duration.count() / 1000;
                //std::string str = std::to_string(spent);
                //str = stateOfTheFace + "  " + str;
                //cv::putText( mat, str.c_str(), cv::Point( face.x, face.y - 10 ), cv::FONT_HERSHEY_SIMPLEX, 1, color, 2 );

                //cv::rectangle( mat, cv::Rect( face.x, face.y, face.width, face.height ), color, 2, 8, 0 );
                cv::rectangle( mat2, cv::Rect( face.x, face.y, face.width, face.height ), color, 2, 8, 0 );
            }
        }

        //outputvideo << mat;
        QImage imageui((const unsigned char *)mat2.data, mat2.cols,mat2.rows,mat2.step, QImage::Format_RGB888);
        emit sigUpdateUI(imageui);

        auto end = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        int spent = duration.count() / 1000;
        if(spent - 50 > 0)
        {
            QThread::msleep(spent - 50);
        }

        if(m_type.type == 2)
        {
            nret = -2;
            m_exited = true;
            break;
        }
    }

    if(m_capture != NULL)
    {
        m_capture->release();
    }

    while(!m_workthread->isFinished())
    {
        QThread::msleep(1);
    }

    emit sigEnd(nret);
}

//return 0:success, -1:src image is invalid, -2:do not find face, 1: find more than one face, 2: quality check failed
int VideoCaptureThread::checkimage(const QString & image, const QString & crop, float * features, SeetaRect &rect)
{
    std::string strimage = image.toStdString();
    std::string strcrop = crop.toStdString();

    cv::Mat mat = cv::imread(strimage.c_str());
    if(mat.empty())
        return -1;

    SeetaImageData img;
    img.width = mat.cols;
    img.height = mat.rows;
    img.channels = mat.channels();
    img.data = mat.data;

    auto face_array = m_fd->detect(img);

    if(face_array.size <= 0)
    {
        return -2;
    }else if(face_array.size > 1)
    {
        return 1;
    }

    SeetaRect& face = face_array.data[0].pos;
    SeetaPointF points[5];

    m_pd->mark(img, face, points);

    m_qa->feed(img, face, points, 5);
    auto result1 = m_qa->query(seeta::BRIGHTNESS);
    auto result2 = m_qa->query(seeta::RESOLUTION);
    auto result3 = m_qa->query(seeta::CLARITY);
    auto result4 = m_qa->query(seeta::INTEGRITY);
    //auto result5 = m_qa->query(seeta::POSE);
    auto result = m_qa->query(seeta::POSE_EX);

    if(result.level == 0 || result1.level == 0 || result2.level == 0 || result3.level == 0 || result4.level == 0 )
    {
        return 2;
    }

    /*
    SeetaPointF points68[68];
    memset( points68, 0, sizeof( SeetaPointF ) * 68 );

    m_pd68->mark(img, face,points68);
    int light, blur, noise;
    light = blur = noise = -1;

    m_lbn->Detect( img, points68, &light, &blur, &noise );
    */
    //std::cout << "light:" << light << ", blur:" << blur << ", noise:" << noise << std::endl;

    seeta::ImageData cropface = m_fr->CropFaceV2(img, points);
    cv::Mat imgmat(cropface.height, cropface.width, CV_8UC(cropface.channels), cropface.data);

    m_fr->ExtractCroppedFace(cropface, features);

    cv::imwrite(strcrop.c_str(), imgmat);

    ///////////////////////////////////////////////
    int x = face.x - face.width / 2;
    if((x) < 0)
        x = 0;
    int y = face.y - face.height / 2;
    if(y  < 0)
        y = 0;

    int x2 = face.x + face.width * 1.5;
    if(x2 >= img.width)
    {
        x2 = img.width -1;
    }

    int y2 = face.y + face.height * 1.5;
    if(y2 >= img.height)
    {
        y2 = img.height -1;
    }

    rect.x = x;
    rect.y = y;
    rect.width = x2 - x;
    rect.height = y2 - y;

    return 0;
}
