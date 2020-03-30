#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDir"
#include "QFileDialog"
#include "QDebug"

#include "qsqlquery.h"
#include "qmessagebox.h"
#include "qsqlerror.h"

#include "qitemselectionmodel.h"
#include <QModelIndex>

//#include "faceinputdialog.h"

#include "inputfilesprocessdialog.h"
#include "resetmodelprocessdialog.h"

#include <QHBoxLayout>
#include <QWidget>
#include <QIntValidator>
#include <QDoubleValidator>

//#include "Common/CStruct.h"
#include <chrono>
using namespace std::chrono;



//////////////////////////////////


const QString gcrop_prefix("crop_");
Config_Paramter gparamters;
std::string gmodelpath;

/////////////////////////////////////
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_currenttab = -1;
    ui->setupUi(this);


    QIntValidator * vfdminfacesize = new QIntValidator(20, 1000);
    ui->fdminfacesize->setValidator(vfdminfacesize);

    QDoubleValidator *vfdthreshold = new QDoubleValidator(0.0,1.0, 2);
    ui->fdthreshold->setValidator(vfdthreshold);

    QDoubleValidator *vantispoofclarity = new QDoubleValidator(0.0,1.0, 2);
    ui->antispoofclarity->setValidator(vantispoofclarity);

    QDoubleValidator *vantispoofreality = new QDoubleValidator(0.0,1.0, 2);
    ui->antispoofreality->setValidator(vantispoofreality);

    QDoubleValidator *vyawhigh = new QDoubleValidator(0.0,90, 2);
    ui->yawhighthreshold->setValidator(vyawhigh);

    QDoubleValidator *vyawlow = new QDoubleValidator(0.0,90, 2);
    ui->yawlowthreshold->setValidator(vyawlow);

    QDoubleValidator *vpitchlow = new QDoubleValidator(0.0,90, 2);
    ui->pitchlowthreshold->setValidator(vpitchlow);

    QDoubleValidator *vpitchhigh = new QDoubleValidator(0.0,90, 2);
    ui->pitchhighthreshold->setValidator(vpitchhigh);

    QDoubleValidator *vfrthreshold = new QDoubleValidator(0.0,1.0, 2);
    ui->fr_threshold->setValidator(vfrthreshold);

    gparamters.MinFaceSize = 100;
    gparamters.Fd_Threshold = 0.80;
    gparamters.VideoWidth = 400;
    gparamters.VideoHeight = 400;
    gparamters.AntiSpoofClarity = 0.30;
    gparamters.AntiSpoofReality = 0.80;
    gparamters.PitchLowThreshold = 20;
    gparamters.PitchHighThreshold = 10;
    gparamters.YawLowThreshold = 20;
    gparamters.YawHighThreshold = 10;
    gparamters.Fr_Threshold = 0.6;
    gparamters.Fr_ModelPath = "face_recognizer.csta";

    m_type.type = 0;
    m_type.filename = "";
    m_type.title = "Open Camera 0";

    ui->recognize_label->setText(m_type.title);

    int width = this->width();
    int height = this->height();
    this->setFixedSize(width, height);

    ui->db_editpicture->setStyleSheet("border-image:url(:/new/prefix1/default.png)");
    ui->db_editcrop->setStyleSheet("border-image:url(:/new/prefix1/default.png)");

    /////////////////////////


    m_database = QSqlDatabase::addDatabase("QSQLITE");
    QString exepath = QCoreApplication::applicationDirPath();
    QString strdb = exepath + /*QDir::separator()*/ + "/seetaface_demo.db";

    m_image_tmp_path = exepath + /*QDir::separator()*/ + "/tmp/";// + QDir::separator();
    m_image_path = exepath + /*QDir::separator()*/ + "/images/";// + QDir::separator();
    //m_model_path = exepath + /*QDir::separator()*/ + "/models/";// + QDir::separator();
    gmodelpath = (exepath + /*QDir::separator()*/ + "/models/"/* + QDir::separator()*/).toStdString();

    QDir dir;
    dir.mkpath(m_image_tmp_path);
    dir.mkpath(m_image_path);

    m_database.setDatabaseName(strdb);

    if(!m_database.open())
    {
        QMessageBox::critical(NULL, "critical", tr("open database failed, exited!"), QMessageBox::Yes);
        exit(-1);
    }

    QStringList tables = m_database.tables();
    m_table = "face_tab";
    m_config_table = "setting_tab";//"paramter_tab";



    bool bfind = false;
    bool bconfigfind = false;
    int i =0;
    for( i=0; i<tables.length(); i++)
    {
        qDebug() << tables[i];
        if(tables[i].compare(m_table) == 0)
        {
            bfind = true;
        }else if(tables[i].compare(m_config_table) == 0)
        {
            bconfigfind = true;
        }
    }

    if(!bfind)
    {
         QSqlQuery query;
        if(!query.exec("create table " +  m_table + " (id int primary key, name varchar(64), image_path varchar(256), feature_data blob, facex int, facey int, facewidth int, faceheight int)"))
        {
            qDebug() << "failed to create table:" + m_table << query.lastError();
            QMessageBox::critical(NULL, "critical", "create table " + m_table + " failed, exited!", QMessageBox::Yes);
            exit(-1);
        }
        qDebug() << "create table ok!";
    }


    if(!bconfigfind)
    {

         QSqlQuery query;
        if(!query.exec("create table " +  m_config_table + " (fd_minfacesize int, fd_threshold real, antispoof_clarity real, antispoof_reality real, qa_yawlow real,qa_yawhigh real,qa_pitchlow real,qa_pitchhigh real,fr_threshold real,fr_modelpath varchar(256))"))
        {
            qDebug() << "failed to create table:" + m_config_table << query.lastError();
            QMessageBox::critical(NULL, "critical", "create table " + m_config_table + " failed, exited!", QMessageBox::Yes);
            exit(-1);
        }

        qDebug() << query.lastQuery();

        QString sql = "insert into " + m_config_table + " (fd_minfacesize, fd_threshold, antispoof_clarity, antispoof_reality, qa_yawlow, qa_yawhigh, qa_pitchlow, qa_pitchhigh,fr_threshold, fr_modelpath) values (";
        sql += "%1, %2, %3, %4, %5, %6, %7, %8, %9, '%10')";
        sql = QString(sql).arg(gparamters.MinFaceSize).arg(gparamters.Fd_Threshold).arg(gparamters.AntiSpoofClarity).arg(gparamters.AntiSpoofReality).
                arg(gparamters.YawLowThreshold).arg(gparamters.YawHighThreshold).arg(gparamters.PitchLowThreshold).arg(gparamters.PitchHighThreshold).
                arg(gparamters.Fd_Threshold).arg(gparamters.Fr_ModelPath);
        qDebug() << sql;
        QSqlQuery q(sql);
        if(!q.exec())
        {
            qDebug() << "insert failed:" << q.lastError();
            QMessageBox::critical(NULL, "critical", "init table " + m_config_table + " failed, exited!", QMessageBox::Yes);
            exit(-1);
        }

        ui->fdminfacesize->setText(QString::number(gparamters.MinFaceSize));
        ui->fdthreshold->setText(QString::number(gparamters.Fd_Threshold));
        ui->antispoofclarity->setText(QString::number(gparamters.AntiSpoofClarity));
        ui->antispoofreality->setText(QString::number(gparamters.AntiSpoofReality));
        ui->yawlowthreshold->setText(QString::number(gparamters.YawLowThreshold));
        ui->yawhighthreshold->setText(QString::number(gparamters.YawHighThreshold));
        ui->pitchlowthreshold->setText(QString::number(gparamters.PitchLowThreshold));
        ui->pitchhighthreshold->setText(QString::number(gparamters.PitchHighThreshold));
        ui->fr_threshold->setText(QString::number(gparamters.Fr_Threshold));
        ui->fr_modelpath->setText(gparamters.Fr_ModelPath);
        qDebug() << "create config table ok!";

    }

    ui->dbtableview->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->dbtableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->dbtableview->verticalHeader()->setDefaultSectionSize(80);
    ui->dbtableview->verticalHeader()->hide();

    connect(ui->dbtableview, SIGNAL(clicked(QModelIndex)), this, SLOT(showfaceinfo()));

    m_model = new QStandardItemModel(this);
    QStringList columsTitles;
    columsTitles << "ID" << "Name" << "Image" << /*"edit" << */" ";
    m_model->setHorizontalHeaderLabels(columsTitles);
    ui->dbtableview->setModel(m_model);
    ui->dbtableview->setColumnWidth(0, 120);
    ui->dbtableview->setColumnWidth(1, 200);
    ui->dbtableview->setColumnWidth(2, 104);
    ui->dbtableview->setColumnWidth(3, 100);
    //ui->dbtableview->setColumnWidth(4, 100);
    getdatas();
    /// ///////////////////////////

    gparamters.VideoWidth = ui->previewlabel->width();
    gparamters.VideoHeight = ui->previewlabel->height();

    if(bconfigfind)
    {
        //fd_minfacesize, fd_threshold, antispoof_clarity, antispoof_reality, qa_yawlow, qa_yawhigh, qa_pitchlow, qa_pitchhigh
        QSqlQuery q("select * from " + m_config_table);
        while(q.next())
        {
            gparamters.MinFaceSize = q.value("fd_minfacesize").toInt();
            ui->fdminfacesize->setText(QString::number(q.value("fd_minfacesize").toInt()));

            gparamters.Fd_Threshold = q.value("fd_threshold").toFloat();
            ui->fdthreshold->setText(QString::number(q.value("fd_threshold").toFloat()));

            gparamters.AntiSpoofClarity = q.value("antispoof_clarity").toFloat();
            ui->antispoofclarity->setText(QString::number(q.value("antispoof_clarity").toFloat()));

            gparamters.AntiSpoofReality = q.value("antispoof_reality").toFloat();
            ui->antispoofreality->setText(QString::number(q.value("antispoof_reality").toFloat()));

            gparamters.YawLowThreshold = q.value("qa_yawlow").toFloat();
            ui->yawlowthreshold ->setText(QString::number(q.value("qa_yawlow").toFloat()));

            gparamters.YawHighThreshold = q.value("qa_yawhigh").toFloat();
            ui->yawhighthreshold ->setText(QString::number(q.value("qa_yawhigh").toFloat()));

            gparamters.PitchLowThreshold = q.value("qa_pitchlow").toFloat();
            ui->pitchlowthreshold ->setText(QString::number(q.value("qa_pitchlow").toFloat()));

            gparamters.PitchHighThreshold = q.value("qa_pitchhigh").toFloat();
            ui->pitchhighthreshold ->setText(QString::number(q.value("qa_pitchhigh").toFloat()));

            gparamters.Fr_Threshold = q.value("fr_threshold").toFloat();
            gparamters.Fr_ModelPath = q.value("fr_modelpath").toString();

            ui->fr_threshold->setText(QString::number(gparamters.Fr_Threshold));
            ui->fr_modelpath->setText(gparamters.Fr_ModelPath);

        }

    }


    ////////////////////////////
    ui->previewtableview->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->previewtableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->previewtableview->verticalHeader()->setDefaultSectionSize(80);
    ui->previewtableview->verticalHeader()->hide();

    //connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(showfaceinfo()));

    m_videomodel = new QStandardItemModel(this);
    columsTitles.clear();
    columsTitles  << "Name" << "Score" << "Gallery" << "Snapshot" << "PID";
    m_videomodel->setHorizontalHeaderLabels(columsTitles);
    ui->previewtableview->setModel(m_videomodel);
    ui->previewtableview->setColumnWidth(0, 140);
    ui->previewtableview->setColumnWidth(1, 80);
    ui->previewtableview->setColumnWidth(2, 84);
    ui->previewtableview->setColumnWidth(3, 84);
    ui->previewtableview->setColumnWidth(4, 2);
    ui->previewtableview->hideColumn(4);

    /////////////////////////
    m_videothread = new VideoCaptureThread(&m_datalst, ui->previewlabel->width(), ui->previewlabel->height());
    m_videothread->setparamter();
    //m_videothread->setMinFaceSize(ui->fdminfacesize->text().toInt());
    connect(m_videothread, SIGNAL(sigUpdateUI(const QImage &)), this, SLOT(onupdateui(const QImage &)));
    connect(m_videothread, SIGNAL(sigEnd(int)), this, SLOT(onvideothreadend(int)));
    connect(m_videothread->m_workthread, SIGNAL(sigRecognize(int, const QString &, const QString &, float, const QImage &, const QRect &)), this,
            SLOT(onrecognize(int, const QString &, const QString &, float, const QImage &, const QRect &)));
    //m_videothread->start();

    m_inputfilesthread = new InputFilesThread(m_videothread, m_image_path, m_image_tmp_path);
    m_resetmodelthread = new ResetModelThread( m_image_path, m_image_tmp_path);

    connect(m_inputfilesthread, SIGNAL(sigInputFilesUpdateUI(std::vector<DataInfo*>*)), this, SLOT(oninputfilesupdateui(std::vector<DataInfo *> *)), Qt::BlockingQueuedConnection);

    ui->dbsavebtn->setEnabled(true);
    ui->previewrunbtn->setEnabled(true);
    ui->previewstopbtn->setEnabled(false);

    //ui->pushButton_6->setEnabled(false);
    ///////////////////////
    ///////////////////////
    //ui->label->setStyleSheet("QLabel{background-color:rgb(255,255,255);}");
    //ui->label->setStyleSheet("border-image:url(:/new/prefix1/white.png)");
    int a = ui->previewlabel->width();
    int b = ui->previewlabel->height();
    QImage image(":/new/prefix1/white.png");
    QImage ime = image.scaled(a,b);
    ui->previewlabel->setPixmap(QPixmap::fromImage(ime));

    ui->tabWidget->setCurrentIndex(0);
    m_currenttab = ui->tabWidget->currentIndex();


    if(m_model->rowCount() > 0)
    {
        ui->dbtableview->scrollToBottom();
        ui->dbtableview->selectRow(m_model->rowCount() - 1);
        emit ui->dbtableview->clicked(m_model->index(m_model->rowCount() - 1, 1));
    }
}

MainWindow::~MainWindow()
{

    delete ui;
    cleardata();
}

void MainWindow::cleardata()
{
    std::map<int, DataInfo *>::iterator iter = m_datalst.begin();
    for(; iter != m_datalst.end(); ++iter)
    {
        if(iter->second)
        {
            delete iter->second;
            iter->second = NULL;
        }
    }
    m_datalst.clear();
}

void MainWindow::getdatas()
{
      int i = 0;
      QSqlQuery q("select * from " + m_table + " order by id  asc");
      while(q.next())
      {
          //qDebug() << q.value("id").toInt() << "-----" << q.value("name").toString() << "----" << q.value("image_path").toString();
          QByteArray data1 = q.value("feature_data").toByteArray();
          float * ptr = (float *)data1.data();
          //qDebug() << ptr[0] << "," << ptr[1] << "," << ptr[2] << "," << ptr[3] ;

          //////////////////////////////////////////////////
          m_model->setItem(i, 0, new QStandardItem(QString::number(q.value("id").toInt())));
          m_model->setItem(i, 1, new QStandardItem(q.value("name").toString()));
         // m_model->setItem(i, 2, new QStandardItem(q.value("image_path").toString()));

          QLabel *label = new QLabel("");
          label->setFixedSize(100,80);
          label->setStyleSheet("border-image:url(" + m_image_path + q.value("image_path").toString() + ")");
          ui->dbtableview->setIndexWidget(m_model->index(m_model->rowCount() - 1, 2), label);

          /*
          QPushButton *button = new QPushButton("edit");
          button->setProperty("id", q.value("id").toInt());
          button->setFixedSize(80, 40);
          connect(button, SIGNAL(clicked()), this, SLOT(editrecord()));
          ui->dbtableview->setIndexWidget(m_model->index(m_model->rowCount() - 1, 3), button);
          */


          QPushButton *button2 = new QPushButton("delete");
          button2->setProperty("id", q.value("id").toInt());
          button2->setFixedSize(80, 40);
          connect(button2, SIGNAL(clicked()), this, SLOT(deleterecord()));

          QWidget *widget = new QWidget();
          QHBoxLayout *layout = new QHBoxLayout;
          layout->addStretch();
          layout->addWidget(button2);
          layout->addStretch();
          widget->setLayout(layout);

          ui->dbtableview->setIndexWidget(m_model->index(m_model->rowCount() - 1, 3), widget);

          //ui->dbtableview->setIndexWidget(m_model->index(m_model->rowCount() - 1, 3), button2);

          DataInfo * info = new DataInfo;
          info->id = q.value("id").toInt();
          info->name = q.value("name").toString();
          info->image_path = q.value("image_path").toString();
          memcpy(info->features, ptr, 1024 * sizeof(float));
          info->x = q.value("facex").toInt();
          info->y = q.value("facey").toInt();
          info->width = q.value("facewidth").toInt();
          info->height = q.value("faceheight").toInt();
          m_datalst.insert(std::map<int, DataInfo *>::value_type(info->id, info));
          i++;
      }
}



void MainWindow::editrecord()
{
    //QPushButton *button = (QPushButton *)sender();
    //qDebug() << button->property("id").toInt() << ", edit";
}

void MainWindow::deleterecord()
{
    QPushButton *button = (QPushButton *)sender();
    qDebug() << button->property("id").toInt() << ",del";
    QMessageBox::StandardButton reply = QMessageBox::question(NULL, "delete", tr("Are you sure delete this record?"), QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::No)
        return;

    QModelIndex modelindex = ui->dbtableview->indexAt(button->pos());

    int id = button->property("id").toInt();
    QStandardItemModel * model = (QStandardItemModel *)ui->dbtableview->model();

    QSqlQuery query("delete from " + m_table + " where id=" + QString::number(id));
    //qDebug() << "delete from " + m_table + " where id=" + QString::number(id);
    if(!query.exec())
    {
        QMessageBox::warning(NULL, "warning", tr("delete this record failed!"), QMessageBox::Yes);
        return;
    }

    int nrows = modelindex.row();
    model->removeRow(modelindex.row());
    std::map<int, DataInfo *>::iterator iter = m_datalst.find(id);
    if(iter != m_datalst.end())
    {
        QFile file(m_image_path + iter->second->image_path);
        file.remove();
        delete iter->second;
        m_datalst.erase(iter);
    }

    if(m_model->rowCount() > 0)
    {
        nrows--;
        if(nrows < 0)
        {
            nrows = 0;
        }
        //qDebug() << "delete------------row:" << nrows;
        ui->dbtableview->selectRow(nrows);
        emit ui->dbtableview->clicked(m_model->index(nrows, 1));
    }else
    {
        ui->db_editname->setText("");
        ui->db_editid->setText("");
        ui->db_editpicture->setStyleSheet("border-image:url(:/new/prefix1/default.png)");
        ui->db_editcrop->setStyleSheet("border-image:url(:/new/prefix1/default.png)");
    }
}

void MainWindow::showfaceinfo()
{
    int row = ui->dbtableview->currentIndex().row();
    //qDebug() << "showfaceinfo:" << row ;
    if(row >= 0)
    {
        QModelIndex index = m_model->index(row, 0);
        int id = ui->db_editid->text().toInt();
        int curid = m_model->data(index).toInt();
        if(id == curid)
            return;


        ui->db_editid->setText(QString::number(m_model->data(index).toInt()));
        std::map<int, DataInfo *>::iterator iter = m_datalst.find(m_model->data(index).toInt());
        if(iter == m_datalst.end())
            return;

        index = m_model->index(row, 1);
        ui->db_editname->setText(m_model->data(index).toString());

        QString strimage = iter->second->image_path;
        //qDebug() << "showfaceinfo:" << strimage;
        ui->db_editpicture->setStyleSheet("border-image:url(" + m_image_path + strimage + ")");


        //qDebug() << "showfaceinfo:" << strimage;
        ui->db_editcrop->setStyleSheet("border-image:url(" + m_image_path + gcrop_prefix + strimage + ")");


        iter = m_datalst.find(id);
        if(iter == m_datalst.end())
            return;
        QFile::remove(m_image_tmp_path + iter->second->image_path);
    }
}

void MainWindow::onrecognize(int pid, const QString & name, const QString & imagepath, float score, const QImage &image, const QRect &rc)
{
    int nrows = m_videomodel->rowCount();

    if(nrows > 1000)
    {
        ui->previewtableview->setUpdatesEnabled(false);
        m_videomodel->removeRows(0, 200);
        ui->previewtableview->setUpdatesEnabled(true);
    }

    nrows = m_videomodel->rowCount();
    int i = 0;
    for(; i<nrows; i++)
    {
        if(m_videomodel->item(i, 4)->text().toInt() == pid)
        {
            break;
        }
    }

    nrows = i;

    m_videomodel->setItem(nrows, 0, new QStandardItem(name));
    //m_videomodel->setItem(nrows, 1, new QStandardItem(QString::number(score, 'f', 3)));

    QLabel *label = new QLabel("");
    label->setFixedSize(80,80);
    if(name.isEmpty())
    {
        m_videomodel->setItem(nrows, 1, new QStandardItem(""));
        label->setText(imagepath);
    }else
    {
        m_videomodel->setItem(nrows, 1, new QStandardItem(QString::number(score, 'f', 3)));
        //QLabel *label = new QLabel("");
        //qDebug() << "rows:" << nrows << ",imagepath:" << imagepath << "," << m_image_path + gcrop_prefix + imagepath ;
        //label->setFixedSize(80,80);

        QImage srcimage;
        srcimage.load( m_image_path + imagepath);
        srcimage = srcimage.copy(rc.x(),rc.y(),rc.width(),rc.height());
        srcimage = srcimage.scaled(80,80);
        label->setPixmap(QPixmap::fromImage(srcimage));
        //label->setStyleSheet("border-image:url(" + m_image_path + gcrop_prefix + imagepath + ")");
        //ui->previewtableview->setIndexWidget(m_videomodel->index(nrows, 2), label);
    }

    ui->previewtableview->setIndexWidget(m_videomodel->index(nrows, 2), label);

    /*
    QLabel *label = new QLabel("");
    qDebug() << "rows:" << nrows << ",imagepath:" << imagepath << "," << m_image_path + gcrop_prefix + imagepath ;
    label->setFixedSize(80,80);

    QImage srcimage;
    srcimage.load( m_image_path + imagepath);
    srcimage = srcimage.copy(rc.x(),rc.y(),rc.width(),rc.height());
    srcimage = srcimage.scaled(80,80);
    label->setPixmap(QPixmap::fromImage(srcimage));
    //label->setStyleSheet("border-image:url(" + m_image_path + gcrop_prefix + imagepath + ")");
    ui->previewtableview->setIndexWidget(m_videomodel->index(nrows, 2), label);
    */

    QLabel *label2 = new QLabel("");
    label2->setFixedSize(80,80);
    QImage img = image.scaled(80,80);
    label2->setPixmap(QPixmap::fromImage(img));
    //label2->setStyleSheet("border-image:url(" + m_image_path + gcrop_prefix + imagepath + ")");
    ui->previewtableview->setIndexWidget(m_videomodel->index(nrows, 3), label2);

    m_videomodel->setItem(nrows, 4, new QStandardItem(QString::number(pid)));
    ui->previewtableview->scrollToBottom();

}

void MainWindow::onupdateui(const QImage & image)
{
    int a = ui->previewlabel->width();
    int b = ui->previewlabel->height();
    QImage ime = image.scaled(a,b);
    ui->previewlabel->setPixmap(QPixmap::fromImage(ime));
    ui->previewlabel->show();
}

void MainWindow::onvideothreadend(int value)
{
    qDebug() << "onvideothreadend:" << value;
    //ui->label->setStyleSheet("border-image:url(:/new/prefix1/white.png)");

    if(m_type.type != 2)
    {
        int a = ui->previewlabel->width();
        int b = ui->previewlabel->height();
        QImage image(":/new/prefix1/white.png");
        QImage ime = image.scaled(a,b);
        ui->previewlabel->setPixmap(QPixmap::fromImage(ime));
        ui->previewlabel->show();
    }

    ui->previewrunbtn->setEnabled(true);
    ui->previewstopbtn->setEnabled(false);
}

void MainWindow::on_dbsavebtn_clicked()
{
    //input image to database
    //phuckDlg *dialog = new phuckDlg(this);
    //dialog->setModal(true);
    //dialog->show();

    //qDebug() << "----begin---update";
    if(ui->db_editname->text().isEmpty())
    {
        QMessageBox::critical(NULL, "critical", tr("name is empty!"), QMessageBox::Yes);
        return;
    }

    if(ui->db_editname->text().length() > 64)
    {
        QMessageBox::critical(NULL, "critical", tr("name length is more than 64!"), QMessageBox::Yes);
        return;
    }

    int index = 1;
    index = ui->db_editid->text().toInt();

    //qDebug() << "----begin---update---index:" << index;
    std::map<int, DataInfo *>::iterator iter = m_datalst.find(index);
    if(iter == m_datalst.end())
    {
        return;
    }

    QString str = m_image_tmp_path + iter->second->image_path;
    QFileInfo fileinfo(str);
    bool imageupdate = false;
    float features[1024];
    SeetaRect rect;

    if(fileinfo.isFile())
    {
        //imageupdate = true;
        QString cropfile = m_image_tmp_path + gcrop_prefix + iter->second->image_path;

        float features[1024];
        int nret = m_videothread->checkimage(str, cropfile, features, rect);
        QString strerror;

        if(nret == -2)
        {
            strerror = "do not find face!";
        }else if(nret == -1)
        {
            strerror = str + " is invalid!";
        }else if(nret == 1)
        {
            strerror = "find more than one face!";
        }else if(nret == 2)
        {
            strerror = "quality check failed!";
        }

        if(!strerror.isEmpty())
        {
            QFile::remove(str);
            QMessageBox::critical(NULL,"critical", strerror, QMessageBox::Yes);
            return;
        }
    }

    //qDebug() << "---1-begin---update---index:" << index;

    QSqlQuery query;

    if(imageupdate)
    {
        query.prepare("update " + m_table + " set name = :name, feature_data=:feature_data, facex=:facex,facey=:facey,facewidth=:facewidth,faceheight=:faceheight where id=" + QString::number(index));
        QByteArray bytearray;
        bytearray.resize(1024 * sizeof(float));
        memcpy(bytearray.data(), features, 1024 * sizeof(float));
        query.bindValue(":feature_data", QVariant(bytearray));
        query.bindValue(":facex", rect.x);
        query.bindValue(":facey", rect.y);
        query.bindValue(":facewidth", rect.width);
        query.bindValue(":faceheight", rect.height);

    }else
    {
        query.prepare("update " + m_table + " set name = :name where id=" + QString::number(index));
    }
    query.bindValue(":name", ui->db_editname->text());//fileinfo.fileName());//strfile);

    if(!query.exec())
    {
        if(imageupdate)
        {
            QFile::remove(str);
            QFile::remove(m_image_tmp_path + gcrop_prefix + iter->second->image_path);
        }

        //QFile::remove()
         //qDebug() << "failed to update table:" << query.lastError();
         QMessageBox::critical(NULL, "critical", tr("update data to database failed!"), QMessageBox::Yes);
         return;
    }

    //qDebug() << "---ddd-begin---update---index:" << index;
    iter->second->name = ui->db_editname->text();


    if(imageupdate)
    {
        memcpy(iter->second->features, features, 1024 * sizeof(float));
        //qDebug() << "---image-begin---update---index:" << index << ",image:" << str;
        QFile::remove(m_image_path + iter->second->image_path);
        QFile::remove(m_image_path + gcrop_prefix + iter->second->image_path);
        QFile::copy(str, m_image_path + iter->second->image_path);
        QFile::copy(m_image_tmp_path + gcrop_prefix + iter->second->image_path, m_image_path + gcrop_prefix + iter->second->image_path);
        QFile::remove(str);
        QFile::remove(m_image_tmp_path + gcrop_prefix + iter->second->image_path);
    }

    int row = ui->dbtableview->currentIndex().row();
    //qDebug() << "showfaceinfo:" << row ;
    if(row >= 0)
    {
        QModelIndex index = m_model->index(row, 1);
        m_model->itemFromIndex(index)->setText(ui->db_editname->text());

        //qDebug() << "---image-begin---update---index:" << index << ",image:" << str;
        if(imageupdate)
        {
            index = m_model->index(row, 2);
            ui->dbtableview->indexWidget(index)->setStyleSheet("border-image:url(" + m_image_path + iter->second->image_path + ")");
            ui->db_editcrop->setStyleSheet("border-image:url(" + m_image_path + gcrop_prefix + iter->second->image_path + ")");
        }
    }
    QMessageBox::information(NULL, "info", tr("update name to database success!"), QMessageBox::Yes);
}

void MainWindow::on_previewrunbtn_clicked()
{
    m_videothread->m_exited = false;
    m_videothread->start(m_type);
    ui->previewrunbtn->setEnabled(false);
    ui->previewstopbtn->setEnabled(true);
}

void MainWindow::on_previewstopbtn_clicked()
{
    m_videothread->m_exited = true;
}

void MainWindow::on_settingsavebtn_clicked()
{
    /*
    ResetModelProcessDlg dialog(this, m_resetmodelthread);
    //m_resetmodelthread->start(&m_datalst, m_table, fr);
    int nret = dialog.exec();

    qDebug() << "ResetModelProcessDlg:" << nret;

    if(nret != QDialog::Accepted)
    {

        QMessageBox::critical(NULL, "critical", "reset face recognizer model failed!", QMessageBox::Yes);
        return;
    }
    return;
    */
    //////////////////////////////////

    int size = ui->fdminfacesize->text().toInt();
    if(size < 20 || size > 1000)
    {
        QMessageBox::warning(NULL, "warn", "Face Detector Min Face Size is invalid!", QMessageBox::Yes);
        return;
    }

    float value = ui->fdthreshold->text().toFloat();
    if(value >= 1.0 || value < 0.0)
    {
        QMessageBox::warning(NULL, "warn", "Face Detector Threshold is invalid!", QMessageBox::Yes);
        return;
    }

    value = ui->antispoofclarity->text().toFloat();
    if(value >= 1.0 || value < 0.0)
    {
        QMessageBox::warning(NULL, "warn", "Anti Spoofing Clarity is invalid!", QMessageBox::Yes);
        return;
    }

    value = ui->antispoofreality->text().toFloat();
    if(value >= 1.0 || value < 0.0)
    {
        QMessageBox::warning(NULL, "warn", "Anti Spoofing Reality is invalid!", QMessageBox::Yes);
        return;
    }

    value = ui->yawlowthreshold->text().toFloat();
    if(value >= 90 || value < 0.0)
    {
        QMessageBox::warning(NULL, "warn", "Quality Yaw Low Threshold is invalid!", QMessageBox::Yes);
        return;
    }
    value = ui->yawhighthreshold->text().toFloat();
    if(value >= 90 || value < 0.0)
    {
        QMessageBox::warning(NULL, "warn", "Quality Yaw High Threshold is invalid!", QMessageBox::Yes);
        return;
    }

    value = ui->pitchlowthreshold->text().toFloat();
    if(value >= 90 || value < 0.0)
    {
        QMessageBox::warning(NULL, "warn", "Quality Pitch Low Threshold is invalid!", QMessageBox::Yes);
        return;
    }
    value = ui->pitchhighthreshold->text().toFloat();
    if(value >= 90 || value < 0.0)
    {
        QMessageBox::warning(NULL, "warn", "Quality Pitch High Threshold is invalid!", QMessageBox::Yes);
        return;
    }

    value = ui->fr_threshold->text().toFloat();
    if(value >= 1.0 || value < 0.0)
    {
        QMessageBox::warning(NULL, "warn", "Face Recognizer Threshold is invalid!", QMessageBox::Yes);
        return;
    }

    QString strmodel = ui->fr_modelpath->text().trimmed();
    QFileInfo fileinfo(gmodelpath.c_str() + strmodel);
    if(QString::compare(fileinfo.suffix(), "csta", Qt::CaseInsensitive) != 0)
    {
        QMessageBox::warning(NULL, "warn", "Face Recognizer model file is invalid!", QMessageBox::Yes);
        return;
    }

    QMessageBox::StandardButton result;
    if(QString::compare(gparamters.Fr_ModelPath, ui->fr_modelpath->text().trimmed()) != 0)
    {
        result = QMessageBox::warning(NULL, "warning", "Set new face recognizer model need reset features, Are you sure?", QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::No)
        {
            return;
        }

        seeta::FaceRecognizer * fr =  m_videothread->CreateFaceRecognizer(ui->fr_modelpath->text().trimmed());
        ResetModelProcessDlg dialog(this, m_resetmodelthread);
        m_resetmodelthread->start(&m_datalst, m_table, fr);
        int nret = dialog.exec();

        qDebug() << "ResetModelProcessDlg:" << nret;

        if(nret != QDialog::Accepted)
        {
            delete fr;
            QMessageBox::critical(NULL, "critical", "reset face recognizer model failed!", QMessageBox::Yes);
            return;
        }
        m_videothread->set_fr(fr);
    }


    QString sql("update " + m_config_table + " set fd_minfacesize=%1, fd_threshold=%2, antispoof_clarity=%3, antispoof_reality=%4,");
    sql += "qa_yawlow=%5, qa_yawhigh=%6, qa_pitchlow=%7, qa_pitchhigh=%8, fr_threshold=%9,fr_modelpath=\"%10\"";
    sql = QString(sql).arg(ui->fdminfacesize->text()).arg(ui->fdthreshold->text()).arg(ui->antispoofclarity->text()).arg(ui->antispoofreality->text()).
            arg(ui->yawlowthreshold->text()).arg(ui->yawhighthreshold->text()).arg(ui->pitchlowthreshold->text()).arg(ui->pitchhighthreshold->text()).
            arg(ui->fr_threshold->text()).arg(ui->fr_modelpath->text().trimmed());
    QSqlQuery q(sql);
    //qDebug() << sql;
    //QSqlQuery q("update " + m_config_table + " set min_face_size =" + ui->fdminfacesize->text() );
    if(!q.exec())
    {
        QMessageBox::critical(NULL, "critical", "update setting failed!", QMessageBox::Yes);
        return;
    }



    gparamters.MinFaceSize = ui->fdminfacesize->text().toInt();
    gparamters.Fd_Threshold = ui->fdthreshold->text().toFloat();
    gparamters.AntiSpoofClarity = ui->antispoofclarity->text().toFloat();
    gparamters.AntiSpoofReality = ui->antispoofreality->text().toFloat();
    gparamters.YawLowThreshold = ui->yawlowthreshold->text().toFloat();
    gparamters.YawHighThreshold = ui->yawhighthreshold->text().toFloat();
    gparamters.PitchLowThreshold = ui->pitchlowthreshold->text().toFloat();
    gparamters.PitchHighThreshold = ui->pitchhighthreshold->text().toFloat();
    gparamters.Fr_Threshold = ui->fr_threshold->text().toFloat();
    gparamters.Fr_ModelPath = ui->fr_modelpath->text().trimmed();

    m_videothread->setparamter();

    QMessageBox::information(NULL, "info", "update setting ok!", QMessageBox::Yes);

}

void MainWindow::on_rotatebtn_clicked()
{
    QMatrix matrix;
    matrix.rotate(90);

    int id = ui->db_editid->text().toInt();

    std::map<int, DataInfo *>::iterator iter = m_datalst.find(id);
    if(iter == m_datalst.end())
    {
        return;
    }

    //QFile::remove(m_image_tmp_path + iter->second->image_path);
    if(!QFile::exists(m_image_tmp_path + iter->second->image_path))
    {
        QFile::copy(m_image_path + iter->second->image_path, m_image_tmp_path + iter->second->image_path);
    }

    if(!QFile::exists(m_image_tmp_path + gcrop_prefix + iter->second->image_path))
    {
        QFile::copy(m_image_path + gcrop_prefix + iter->second->image_path, m_image_tmp_path + gcrop_prefix + iter->second->image_path);
    }
    //QFile::copy(m_image_path + iter->second->image_path, m_image_tmp_path + iter->second->image_path);

    QImage image(m_image_tmp_path + iter->second->image_path);
    if(image.isNull())
        return;

    image = image.transformed(matrix, Qt::FastTransformation);
    image.save(m_image_tmp_path + iter->second->image_path);

    ui->db_editpicture->setStyleSheet("border-image:url(" + m_image_tmp_path + iter->second->image_path + ")");

    ///////////////////////
    //QMatrix cropmatrix;
    matrix.reset();
    matrix.rotate(90);
    QImage cropimage(m_image_tmp_path + gcrop_prefix + iter->second->image_path);
    if(cropimage.isNull())
        return;

    cropimage = cropimage.transformed(matrix, Qt::FastTransformation);
    cropimage.save(m_image_tmp_path + gcrop_prefix + iter->second->image_path);

    ui->db_editcrop->setStyleSheet("border-image:url(" + m_image_tmp_path + gcrop_prefix + iter->second->image_path + ")");

}



void MainWindow::on_tabWidget_currentChanged(int index)
{
    //qDebug() <<  "cur:" << ui->tabWidget->tabText(index) << ",old:" << ui->tabWidget->tabText(m_currenttab) ;
    if(m_currenttab != index)
    {
        if(m_currenttab == 2)
        {
           on_previewstopbtn_clicked();
           m_videothread->wait();
        }
        m_currenttab = index;
    }
    //qDebug() <<  "tab:" << ui->tabWidget->tabText(index) << ",cur:" << index << ",old:" << ui->tabWidget->currentIndex();
}

void MainWindow::on_addimagebtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("open image file"),
                                                         "./" ,
                                                        "JPEG Files(*.jpg *.jpeg);;PNG Files(*.png);;BMP Files(*.bmp)");
    //qDebug() << "image:" << fileName;

    QImage image(fileName);
    if(image.isNull())
        return;

    QFile file(fileName);
    QFileInfo fileinfo(fileName);

    //////////////////////////////
    QSqlQuery query;
    query.prepare("insert into  " + m_table + " (id, name, image_path, feature_data, facex,facey,facewidth,faceheight) values (:id, :name, :image_path, :feature_data,:facex,:facey,:facewidth,:faceheight)");

    int index = 1;
    if(m_model->rowCount() > 0)
    {
        index = m_model->item(m_model->rowCount() - 1, 0)->text().toInt() + 1;
    }


    QString strfile = QString::number(index) + "_" + fileinfo.fileName();//m_image_path + QString::number(index) + "_" + m_currentimagefile;//fileinfo.fileName();

    QString cropfile = m_image_path + gcrop_prefix + strfile;

    float features[1024];
    SeetaRect rect;
    int nret = m_videothread->checkimage(fileName, cropfile, features, rect);
    QString strerror;

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
        QMessageBox::critical(NULL,"critical", strerror, QMessageBox::Yes);
        return;
    }

    QString name = fileinfo.completeBaseName();//fileName();
    int n = name.indexOf("_");

    if(n >= 1)
    {
        name = name.left(n);
    }

    query.bindValue(0, index);
    query.bindValue(1,name);

    //query.bindValue(2, "/wqy/Downloads/ap.jpeg");
    query.bindValue(2, strfile);//fileinfo.fileName());//strfile);

    //float data[4] = {0.56,0.223,0.5671,-0.785};
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
         QMessageBox::critical(NULL, "critical", tr("save face data to database failed!"), QMessageBox::Yes);
         return;
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
    m_datalst.insert(std::map<int, DataInfo *>::value_type(index, info));

    ////////////////////////////////////////////////////////////
    int rows = m_model->rowCount();
    //qDebug() << "rows:" << rows;

    m_model->setItem(rows, 0, new QStandardItem(QString::number(index)));
    m_model->setItem(rows, 1, new QStandardItem(info->name));

    QLabel *label = new QLabel("");

    label->setStyleSheet("border-image:url(" + m_image_path + strfile + ")");
    ui->dbtableview->setIndexWidget(m_model->index(rows, 2), label);

    QPushButton *button2 = new QPushButton("delete");
    button2->setProperty("id", index);
    button2->setFixedSize(80, 40);
    connect(button2, SIGNAL(clicked()), this, SLOT(deleterecord()));

    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(button2);
    layout->addStretch();
    widget->setLayout(layout);

    ui->dbtableview->setIndexWidget(m_model->index(rows, 3), widget);
    ui->dbtableview->scrollToBottom();
    ui->dbtableview->selectRow(rows);

    emit ui->dbtableview->clicked(m_model->index(rows, 1));
    //QMessageBox::information(NULL, "info", tr("add face operator success!"), QMessageBox::Yes);

}

void MainWindow::on_menufacedbbtn_clicked()
{
    ui->tabWidget->setCurrentIndex(1);
}



void MainWindow::on_menusettingbtn_clicked()
{

     ui->tabWidget->setCurrentIndex(3);
}

void MainWindow::on_previewclearbtn_clicked()
{
    ui->previewtableview->setUpdatesEnabled(false);
    m_videomodel->removeRows(0, m_videomodel->rowCount());
    //m_videomodel->clear();
    ui->previewtableview->setUpdatesEnabled(true);
}

void MainWindow::on_menuopenvideofile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("open video file"),
                                                         "./" ,
                                                        "MP4 Files(*.mp4 *.MP4);;AVI Files(*.avi);;FLV Files(*.flv);;h265 Files(*.h265);;h263 Files(*.h263)");
    //qDebug() << "image:" << fileName;
    m_type.type = 1;
    m_type.filename = fileName;
    m_type.title = "Open Video: " + fileName;
    ui->recognize_label->setText(m_type.title);
    ui->tabWidget->setCurrentIndex(2);
    emit ui->previewrunbtn->clicked();
}

void MainWindow::on_menuopenpicturefile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("open image file"),
                                                         "./" ,
                                                        "JPEG Files(*.jpg *.jpeg);;PNG Files(*.png);;BMP Files(*.bmp)");
    //qDebug() << "image:" << fileName;
    m_type.type = 2;
    m_type.filename = fileName;
    m_type.title = "Open Image: " + fileName;
    ui->recognize_label->setText(m_type.title);
    ui->tabWidget->setCurrentIndex(2);
    emit ui->previewrunbtn->clicked();
}

void MainWindow::on_menuopencamera_clicked()
{
    m_type.type = 0;
    m_type.filename = "";
    m_type.title = "Open Camera: 0";
    ui->recognize_label->setText(m_type.title);
    ui->tabWidget->setCurrentIndex(2);
    emit ui->previewrunbtn->clicked();
}

static void FindFile(const QString & path, QStringList &files)
{
    QDir dir(path);
    if(!dir.exists())
        return;

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setSorting(QDir::DirsFirst);;

    QFileInfoList list = dir.entryInfoList();
    int i = 0;
    while(i < list.size())
    {
        QFileInfo info = list.at(i);
        //qDebug() << info.absoluteFilePath();
        if(info.isDir())
        {
            FindFile(info.absoluteFilePath(), files);
        }else
        {
            QString str = info.suffix();
            if(str.compare("png", Qt::CaseInsensitive) == 0 || str.compare("jpg", Qt::CaseInsensitive) == 0 || str.compare("jpeg", Qt::CaseSensitive) == 0 || str.compare("bmp", Qt::CaseInsensitive) == 0)
            {
                files.append(info.absoluteFilePath());
            }
        }
        i++;
    }
    return;
}

void MainWindow::on_addfilesbtn_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Select Directorky"), ".");
    if(fileName.isEmpty())
    {
        return;
    }

    qDebug() << fileName;
    QStringList files;
    FindFile(fileName, files);
     qDebug() << files.size();
    if(files.size() <= 0)
        return;

    for(int i=0; i<files.size(); i++)
    {
        qDebug() << files.at(i);
    }

    qDebug() << files.size();
    //return;
    int index = 1;
    if(m_model->rowCount() > 0)
    {
        index = m_model->item(m_model->rowCount() - 1, 0)->text().toInt();
    }

    InputFilesProcessDlg dialog(this, m_inputfilesthread);


    m_inputfilesthread->start(&files, index, m_table);
    dialog.exec();


    //qDebug() << "------on_addfilesbtn_clicked---end";
}

void MainWindow::oninputfilesupdateui(std::vector<DataInfo *> * datas)
{
    DataInfo * info = NULL;
    //qDebug() << "----oninputfilesupdateui--" << datas->size();
    if(datas->size() > 0)
    {
        ui->dbtableview->setUpdatesEnabled(false);
    }

    int rows = 0;
    for(int i=0; i<datas->size(); i++)
    {
        rows = m_model->rowCount();
        //qDebug() << "rows:" << rows;
        info = (*datas)[i];
        m_datalst.insert(std::map<int, DataInfo *>::value_type(info->id, info));
        m_model->setItem(rows, 0, new QStandardItem(QString::number(info->id)));
        m_model->setItem(rows, 1, new QStandardItem(info->name));

        QLabel *label = new QLabel("");

        label->setStyleSheet("border-image:url(" + m_image_path + info->image_path + ")");
        ui->dbtableview->setIndexWidget(m_model->index(rows, 2), label);

        QPushButton *button2 = new QPushButton("delete");
        button2->setProperty("id", info->id);
        button2->setFixedSize(80, 40);
        connect(button2, SIGNAL(clicked()), this, SLOT(deleterecord()));
        ui->dbtableview->setIndexWidget(m_model->index(rows, 3), button2);
        //ui->dbtableview->scrollToBottom();
        //ui->dbtableview->selectRow(rows);
    }
    if(datas->size() > 0)
    {
        ui->dbtableview->setUpdatesEnabled(true);
        ui->dbtableview->scrollToBottom();
        ui->dbtableview->selectRow(rows);
        emit ui->dbtableview->clicked(m_model->index(rows, 1));
    }

}

void MainWindow::on_settingselectmodelbtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("open model file"),
                                                         "./" ,
                                                        "CSTA Files(*.csta)");
    QFileInfo fileinfo(fileName);
    QString modelfile = fileinfo.fileName();

    QString str = gmodelpath.c_str() + modelfile;

    qDebug() << "------str:" << str;
    qDebug() << "fileName:" << fileName;

    if(QString::compare(fileName, str) == 0)
    {
        ui->fr_modelpath->setText(modelfile);
        return;
    }
    //QFile file(fileName);
    if(!QFile::copy(fileName, str))
    {
        QMessageBox::critical(NULL, "critical", "Copy model file: " + fileName + " to " + gmodelpath.c_str() +  " failed, file already exists!", QMessageBox::Yes);
        return;
    }

    ui->fr_modelpath->setText(modelfile);

    //m_videothread->reset_fr_model(modelfile);
    //qDebug() << "image:" << fileName;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_videothread->m_exited = true;
    m_videothread->wait();
    QWidget::closeEvent(event);
}
