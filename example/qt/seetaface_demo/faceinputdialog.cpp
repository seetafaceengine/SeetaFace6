#include<QtGui>
#include<QLabel>
#include<QLineEdit>
#include<QCheckBox>
#include<QPushButton>
#include<QHBoxLayout>
#include "faceinputdialog.h"


#include "QDir"
#include "QFileDialog"
#include "QDebug"

#include "qsqlquery.h"
#include "qmessagebox.h"
#include "qsqlerror.h"

phuckDlg::phuckDlg(QWidget *parent)
    : QDialog(parent)
{

    qDebug() << "------------dlg input----------------";
    //初始化控件对象
    //tr是把当前字符串翻译成为其他语言的标记
    //&后面的字母是用快捷键来激活控件的标记，例如可以用Alt+w激活Find &what这个控件
    label1 = new QLabel(tr("name:"));//QLabel(tr("Find &what:"));
    edit1 = new QLineEdit;
    QLabel *labeltmp = new QLabel(tr(""));//QLabel(tr("Find &what:"));
    //设置伙伴控件
    label1->setBuddy(edit1);

    QLabel *label2;
    QLabel *image_label;

    QLabel *label3;
    QLabel *image_crop_label;


    label2 = new QLabel(tr("picture:"));//QLabel(tr("Find &what:"));

    image_label = new QLabel(tr(""));
    image_label->setFixedSize(240, 240);
    QImage img(":/new/prefix1/default.png");

    img = img.scaled(240,240);

    m_defaultimage1 = img;//QPixmap::fromImage(img);
    m_currentimage = m_defaultimage1;
    image_label->setPixmap(QPixmap::fromImage(img));

    label3 = new QLabel(tr("face:"));//QLabel(tr("Find &what:"));

    image_crop_label = new QLabel(tr(""));
    image_crop_label->setFixedSize(100, 100);

    img = img.scaled(100,100);
    m_defaultimage2 = img;//QPixmap::fromImage(img);
    image_crop_label->setPixmap(QPixmap::fromImage(img));

    //box1 = new QCheckBox(tr("Mach &case"));
    //box2 = new QCheckBox(tr("Search &background"));
    okButton = new QPushButton(tr("&Save"));
    okButton->setDefault(true);
    okButton->setEnabled(false);

    closeButton = new QPushButton(tr("&Close"));


    inputButton = new QPushButton(tr("&Select Picture ..."));
    rotateButton = new QPushButton(tr("&Rotate"));
    //连接信号和槽
    connect(edit1, SIGNAL(textChanged()), this, SLOT(enableOkButton()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(inputButton, SIGNAL(clicked()), this, SLOT(inputClicked()));
    connect(rotateButton, SIGNAL(clicked()), this, SLOT(rotateClicked()));

    //设置控件位置
    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(label1);
    topLeftLayout->addWidget(edit1);
    topLeftLayout->addWidget(labeltmp);


    QHBoxLayout *topLeftLayout2 = new QHBoxLayout;
    topLeftLayout2->addWidget(label2);
    topLeftLayout2->addWidget(image_label);

    QHBoxLayout *topLeftLayout3 = new QHBoxLayout;
    topLeftLayout3->addWidget(label3);
    topLeftLayout3->addWidget(image_crop_label);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(topLeftLayout3);
    layout->addWidget(inputButton);
    layout->addWidget(rotateButton);
    //leftLayout->addWidget(box1);
    //leftLayout->addWidget(box2);

    topLeftLayout2->addLayout(layout);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(closeButton);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLeftLayout);
    mainLayout->addLayout(topLeftLayout2);
    mainLayout->addLayout(bottomLayout);

    this->setLayout(mainLayout);

    setWindowTitle(tr("face input"));

    setFixedHeight(sizeHint().height());
}

void phuckDlg::okClicked(){
    QString text1 = edit1->text();
    /*
    Qt::CaseSensitivity cs;
    if (box1->isChecked()){
        cs = Qt::CaseSensitive;
    }
    else{
        cs = Qt::CaseInsensitive;
    }

    //判断发送什么信号
    if (box2->isChecked()){
        emit findPrevious(text1, cs);
    }
    else{
        emit findNext(text1, cs);
    }
    */
}

void phuckDlg::inputClicked(){

    QString fileName = QFileDialog::getOpenFileName(this, tr("open image file"),
                                                         "./" ,
                                                        "JPEG Files(*.jpg *.jpeg);;PNG Files(*.png);;BMP Files(*.bmp)");
    qDebug() << "image:" << fileName;

    QImage image(fileName);
    if(image.isNull())
    {
        qDebug() << "imag  222e:" << fileName;
        QMessageBox::warning(NULL, "warnning", fileName + " is invalid image file!", QMessageBox::Yes);
        return;
    }

    qDebug() << "image222:" << fileName;

    image = image.scaled(240,240);
    m_currentimage =image;// QPixmap::fromImage(image);
    image_label->setPixmap(QPixmap::fromImage(image));

    m_imagefile = fileName;
    //QDir dir(fileName);

}

void phuckDlg::rotateClicked(){
    QMatrix matrix;
    matrix.rotate(90);

    //QImage image(m_imagefile);
    //m_currentimage = m_currentimage.transformed(matrix, Qt::FastTransformation);

    m_currentimage = m_currentimage.transformed(matrix, Qt::FastTransformation);
    image_label->setPixmap(QPixmap::fromImage(m_currentimage));


}

void phuckDlg::enableOkButton(){
    //okButton->setEnabled();
}

phuckDlg::~phuckDlg()
{

}

