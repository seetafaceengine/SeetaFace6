#ifndef RESETMODELPROCESSDIALOG_H
#define RESETMODELPROCESSDIALOG_H



#include<QDialog>


class QLabel;
class QProgressBar;
class QPushButton;
class ResetModelThread;

class ResetModelProcessDlg :public QDialog{

    //如果需要在对话框类中自定义信号和槽，则需要在类内添加Q_OBJECT
    Q_OBJECT
public:
    //构造函数，析构函数
    ResetModelProcessDlg(QWidget *parent, ResetModelThread * thread);
    ~ResetModelProcessDlg();

protected:
    void closeEvent(QCloseEvent *event);
    //在signal和slots中定义这个对话框所需要的信号。
signals:
    //signals修饰的函数不需要本类实现。他描述了本类对象可以发送那些求助信号

//slots必须用private修饰
private slots:
    void cancelClicked();
    void setProgressValue(float value);
    void setResetModelEnd(int);
//申明这个对话框需要哪些组件
private:
    QLabel *label;

    QProgressBar *progressbar;
    //QLabel *label2;

    QPushButton *cancelButton;//, *closeButton;

    ResetModelThread * workthread;
    bool m_exited;
};




#endif // RESETMODELPROCESSDIALOG_H
