#include "mythread.h"
#include "mainwindow.h"
MyThread::MyThread()
{

}
//extern QString posstr;
void    MyThread::run ()
{
   // Downprc("1");
     //......省略一些分析网络数据的代码
//发射一个Log信号，这样主线程就可以安全的对界面进行修改了
   // while (1) {
  //      emit   Log(QString(posstr));
  //  }

}
