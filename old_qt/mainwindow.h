#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <libusb-1.0/libusb.h>

#include <QMainWindow>
#include <QtWidgets>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
public:
    int Open_Device();
    int downloadProgress();
    bool writedata(libusb_device_handle *lbdev, uchar *pfBin, int cmdlength, int datalength);
    bool ReadBackCheckdata(libusb_device_handle *lbdev, uchar *pfBin, int cmdlength, int datalength);
    bool writeSerialNumber(libusb_device_handle *handleDev, int cmdlength);
public slots:  //这里制造一个名为Log的回调（槽），这个回调会对界面的一个QTextEdit控件追加一行文字（参数sMessage）
void    Log                 (QString   sMessage);

};



#define VID 0x258a
#define PID 0x001e

#define BOOTVID 0x0603
#define BOOTPID 0x1020

#define BOOTMODE 0xFE
#define USERMODE 0xFD

//extern libusb_device_handle *devh;
//extern libusb_context *ctx;

//void setlabeltxt(QString st);

#define binlen (14*1024)

int Downprc(QString name);
int HexToBinFile(int &length, uchar* pbuf);

#endif // MAINWINDOW_H
