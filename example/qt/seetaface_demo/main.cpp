#include "mainwindow.h"
#include <QApplication>
#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"))
    MainWindow w;
    w.setWindowTitle("SeetaFace Demo");
    w.setWindowIcon(QIcon(":/new/prefix1/seetatech_logo.png"));
    w.show();

    QString str("乱码");

    qDebug() << str;
    return a.exec();
}
