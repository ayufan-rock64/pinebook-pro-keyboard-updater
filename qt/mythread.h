#ifndef MYTHREAD_H
#define MYTHREAD_H
#include    <QThread>

class MyThread : public QThread
{
Q_OBJECT
public:
    MyThread();

public:
         virtual    void    run ();
signals:     //这里制造一个名为Log的信号
        void    Log                 (QString   sMessage);
};

#endif // MYTHREAD_H
