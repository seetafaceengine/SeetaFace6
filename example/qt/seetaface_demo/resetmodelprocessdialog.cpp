#include<QtGui>
#include<QLabel>
#include<QProgressBar>
#include<QPushButton>
#include<QHBoxLayout>
#include "resetmodelprocessdialog.h"

#include "videocapturethread.h"


ResetModelProcessDlg::ResetModelProcessDlg(QWidget *parent, ResetModelThread * thread)
    : QDialog(parent)
{
    m_exited = false;
    workthread = thread;
    qDebug() << "------------dlg input----------------";
    //初始化控件对象
    //tr是把当前字符串翻译成为其他语言的标记
    //&后面的字母是用快捷键来激活控件的标记，例如可以用Alt+w激活Find &what这个控件
    label  = new QLabel("", this);

    progressbar = new QProgressBar(this);
    progressbar->setOrientation(Qt::Horizontal);
    progressbar->setMinimum(0);
    progressbar->setMaximum(100);
    progressbar->setValue(5);
    progressbar->setFormat(tr("current progress:%1%").arg(QString::number(5, 'f',1)));
    progressbar->setAlignment(Qt::AlignLeft| Qt::AlignVCenter);

    cancelButton = new QPushButton(tr("&Cancel"));
    //cancelButton->setEnabled(true);

    //closeButton = new QPushButton(tr("&Close"));


    //连接信号和槽
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));
    //connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    //connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(workthread, SIGNAL(sigprogress(float)), this, SLOT(setProgressValue(float)));
    connect(workthread, SIGNAL(sigResetModelEnd(int)), this, SLOT(setResetModelEnd(int)));



    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(cancelButton);
    //bottomLayout->addWidget(closeButton);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(label);
    mainLayout->addWidget(progressbar);
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    this->setLayout(mainLayout);

    setWindowTitle(tr("Reset Face Recognizer Model Progress"));

    //cancelButton->setEnabled(true);
    setFixedSize(400,160);
}

void ResetModelProcessDlg::closeEvent(QCloseEvent *event)
{
    if(!m_exited)
    {
        workthread->m_exited = true;
        event->ignore();
    }else
    {
        event->accept();
    }
}

void ResetModelProcessDlg::cancelClicked()
{
    qDebug() << "ResetModelProcessDlg cancelclicked";
    workthread->m_exited = true;
}


ResetModelProcessDlg::~ResetModelProcessDlg()
{
    qDebug() << "ResetModelProcessDlg ~ResetModelProcessDlg";
}
void ResetModelProcessDlg::setResetModelEnd(int value)
{
    m_exited = true;
    this->hide();
    qDebug() << "setResetModelEnd:" << value;
    if(value == 0)
    {
        accept();
    }else
    {
        reject();
    }

}


void ResetModelProcessDlg::setProgressValue(float value)
{
     QString str = QString("%1%").arg(QString::number(value, 'f',1));
     progressbar->setValue(value);
     progressbar->setFormat(str);
}
