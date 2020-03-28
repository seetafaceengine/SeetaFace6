#ifndef FACEINPUTDIALOG_H
#define FACEINPUTDIALOG_H


#include<QDialog>
#include<QPixmap>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;


class phuckDlg :public QDialog{

    //如果需要在对话框类中自定义信号和槽，则需要在类内添加Q_OBJECT
    Q_OBJECT
public:
    //构造函数，析构函数
    phuckDlg(QWidget *parent = NULL);
    ~phuckDlg();

    //在signal和slots中定义这个对话框所需要的信号。
signals:
    //signals修饰的函数不需要本类实现。他描述了本类对象可以发送那些求助信号
    //void findNext(const QString& str1, Qt::CaseSensitivity cs);
    //void findPrevious(const QString& str1, Qt::CaseSensitivity cs);

//slots必须用private修饰
private slots:
    void okClicked();
    void enableOkButton();

    void inputClicked();
    void rotateClicked();
//申明这个对话框需要哪些组件
private:
    QLabel *label1;
    QLineEdit *edit1;

    QLabel *label2;
    QLabel *image_label;

    QLabel *label3;
    QLabel *image_crop_label;
    //QCheckBox *box1, *box2;
    QPushButton *okButton, *closeButton, *rotateButton, *inputButton;
    QImage m_defaultimage1;
    QImage m_defaultimage2;
    QImage m_currentimage;

    QString m_imagefile;

    QString m_name;
    QString m_imagepath;

    int     m_id;
    float   m_features[1024];
};




#endif // FACEINPUTDIALOG_H
