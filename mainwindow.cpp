#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <unistd.h>
#include <QThread>
#include <QString>
#include <QtConcurrent>
#include <QtWidgets/QMessageBox>
#include <stdlib.h>
#include <stdio.h>
#include <qdebug.h>
#include "mythread.h"
// MyThread Thread;
// QString posstr="0%";

bool Is_bootusb_connet = false;
libusb_device_handle *devh;
libusb_context *ctx;

//---------------------------------
uchar *binfile =new uchar[binlen];


quint16 m_serial_number=1;
int pos = 0;
bool isdown =false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

 //   QObject::connect(&Thread, SIGNAL(Log(QString)), this, SLOT(Log(QString)));

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{

    delete ui;
}

void   MainWindow:: Log                 (QString   sMessage) {
   // m_qeLog.append(sMessage);
    //ui->label_3->setText(sMessage);
}

//


//----------------------------------




static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    struct libusb_device_descriptor desc;
    int rc;

    (void)ctx;
    (void)dev;
    (void)event;
    (void)user_data;

    if(isdown==false)
    {
        isdown = true;
        rc = libusb_get_device_descriptor(dev, &desc);
        if (LIBUSB_SUCCESS != rc) {
            fprintf (stderr, "Error getting device descriptor\n");
        }

        printf ("Device attached: %04x:%04x\n", desc.idVendor, desc.idProduct);

        if((desc.idProduct==BOOTPID)&&(desc.idVendor==BOOTVID))
        {
            libusb_close(devh);
            libusb_exit(ctx);
            usleep(100000);
            libusb_init(&ctx);
            Is_bootusb_connet = true;
            devh = libusb_open_device_with_vid_pid(ctx, BOOTVID, BOOTPID);

            if(devh == NULL)
            {
                printf("OPEN ERR\r\n");
                return 0;
            }

            rc = libusb_kernel_driver_active(devh, 0);
            if (rc == 1)
            {
                printf("Kernel Driver Active\n");
                rc = libusb_detach_kernel_driver(devh, 0);
                if ( rc == 0)
                    printf("Kernel Driver Detached\n");
                else
                    return rc;
            }
            else
            {
                return rc;
            }

            rc = libusb_claim_interface(devh, 0);
            printf("usb boot\r\n");
            fflush(stdout);

           // Thread.start();

            QFuture<void> fut2 = QtConcurrent::run(Downprc, QString("Thread 2"));

            fut2.waitForFinished();

        }
    }

   /* if (devh) {
        libusb_close (devh);
        devh = NULL;
    }*/

    return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    (void)ctx;
    (void)dev;
    (void)event;
    (void)user_data;

    printf ("Device detached\n");

    if (devh) {
        libusb_close (devh);
        libusb_exit(ctx);
        devh = NULL;
    }

//	done++;

    return 0;
}



int Hotusb(QString name)
{
    int rc;

    while (Is_bootusb_connet==false)
    {
        rc = libusb_handle_events (NULL);
        printf("hotusb %d\n",rc);
        fflush(stdout);
        if (rc < 0)
            printf("libusb_handle_events() failed: %s\n", libusb_error_name(rc));
    }
    return 0;
}


void MainWindow::on_pushButton_clicked()
{
    isdown = false;
    int rc = Open_Device();
    printf("open device-->mode %d\n",rc);
    fflush(stdout);
    if(rc == USERMODE)
    {
        printf("device in user mode ,goto bootmode\r\n");
        libusb_hotplug_callback_handle hp[2];
       // int product_id, vendor_id, class_id;
        int rc;

        unsigned char dataOut[0x30]={0};
        dataOut[0]=0x05;
        dataOut[1]=0x75;
        dataOut[2]=0x00;
        dataOut[3]=0x00;
        dataOut[4]=0x00;
        dataOut[5]=0x00;
        libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 1, dataOut, 6, 100);

        rc = libusb_init (NULL);
        if (rc < 0)
        {
            printf("failed to initialise libusb: %s\n", libusb_error_name(rc));
            return ;
        }

        if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
            printf ("Hotplug capabilites are not supported on this platform\n");
            libusb_exit (NULL);
            return ;
        }

        rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, LIBUSB_HOTPLUG_NO_FLAGS, BOOTVID,
            BOOTPID, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL, &hp[0]);
        if (LIBUSB_SUCCESS != rc) {
            fprintf (stderr, "Error registering callback 0\n");
            libusb_exit (NULL);
            return ;
        }

        rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_NO_FLAGS, BOOTVID,
            BOOTPID,LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback_detach, NULL, &hp[1]);
        if (LIBUSB_SUCCESS != rc) {
            fprintf (stderr, "Error registering callback 1\n");
            libusb_exit (NULL);
            return ;

        }
        QFuture<void> fut1 = QtConcurrent::run(Hotusb, QString("Thread 1"));
        fut1.waitForFinished();
    }
    else if(rc == BOOTMODE)
    {
        printf("device in boot mode\n");

        fflush(stdout);
      //  Thread.start();
        QFuture<void> fut2 = QtConcurrent::run(Downprc, QString("Thread 2"));

        fut2.waitForFinished();
    }
}

int MainWindow::Open_Device()
{

    int rc;
    int mode;
    int indf;

    if (devh) {
        printf("release interface\r\n");
        libusb_release_interface(devh, 0); //释放接口
        libusb_close (devh);
        libusb_exit(ctx);
        devh = NULL;
    }

    rc = libusb_init(&ctx);

    if(rc<0)
        return rc;

    devh = libusb_open_device_with_vid_pid(ctx, VID, PID);

    if(devh == NULL)
    {
        devh = libusb_open_device_with_vid_pid(ctx, BOOTVID, BOOTPID);
        if(devh == NULL)
            return -1;
        else
        {
            indf = 0;
            mode = BOOTMODE;  //boot mode
        }
    }
    else
    {
        indf = 1;
        mode = USERMODE; //user mode
    }

    rc = libusb_kernel_driver_active(devh, indf);
    if (rc == 1)
    {
        printf("Kernel Driver Active\n");
        rc = libusb_detach_kernel_driver(devh, indf);
        if ( rc == 0)
            printf("Kernel Driver Detached\n");
        else
            return rc;
    }
    else
    {
        return rc;
    }

    rc = libusb_claim_interface(devh, indf);

    if(rc < 0)
        return rc;

    return mode;
}


//----------------download


int Downprc(QString name)
{
    int len; 
    MainWindow mw;

    if(HexToBinFile(len,binfile)==2)
    {
        int dw = mw.downloadProgress();

        if (dw == 0)
        {
            printf("update pass\r\n");
            libusb_release_interface(devh, 0); //释放接口
            libusb_close(devh);  //关闭设备
            libusb_exit(ctx);  //退出libusb上下文
            //pass
        }
        else if(dw == 2)
        {
            printf("update fail\r\n");
            libusb_release_interface(devh, 0); //释放接口
            libusb_close(devh);  //关闭设备
            libusb_exit(ctx);  //退出libusb上下文
            //fail
        }
        return 0;
    }

    printf("Downpr\r\n");
    fflush(stdout);
    return 0;
}

int HexToBinFile(int &length, uchar* pbuf)
{
    FILE* in;
    if((in =fopen("fw.hex","rt"))==NULL)
    {
        QMessageBox::critical(0 ,
        "Error" , "Can not finded hex file",
        QMessageBox::Ok,
        0 ,  0 );

        return 1;
    }

    uchar* pbuffer;
    pbuffer = new uchar[binlen];
    memset(pbuffer, 0, binlen);

    char endstr[20];
    strcpy(endstr, ":00000001FF\n");


    char strbuf[256];
    char *endptr;

    int RealLength = 0;

    while (1)
    {
        if (fgets(strbuf, 256, in) == NULL)
        {
            break;
        }

        if (strcmp(endstr, strbuf) == 0)
        {
            break;
        }

        char len_str[3];
        len_str[0] = strbuf[1];
        len_str[1] = strbuf[2];
        len_str[2] = 0;
        int len = strtol(len_str, &endptr, 16);

        char addr_str[5];
        addr_str[0] = strbuf[3];
        addr_str[1] = strbuf[4];
        addr_str[2] = strbuf[5];
        addr_str[3] = strbuf[6];
        addr_str[4] = 0;
        int addr = strtol(addr_str, &endptr, 16);


        char val_str[3];
        val_str[2] = 0;
        for (int i = 0; i < len; i++)
        {
            val_str[0] = strbuf[2 * i + 9];
            val_str[1] = strbuf[2 * i + 1 + 9];
            int val = strtol(val_str, &endptr, 16);
            if (addr >= 14 * 1024)
            {
                break;
            }
            else
            {
                *(pbuffer + addr) = val;
            }
            addr++;
            if (addr > RealLength)
            {
                RealLength = addr;
            }
        }
    }

    fclose(in);

    uchar tempBuff[3];

    for (int i = 0; i < RealLength; i++)
    {
        *(pbuf + i) = *(pbuffer + i);

        if (i == 0x37FB)
        {
            tempBuff[0] = *(pbuffer + i);
        }

        if (i == 0x37FC)
        {
            tempBuff[1] = *(pbuffer + i);
        }

        if (i == 0x37FD)
        {
            tempBuff[2] = *(pbuffer + i);
        }
    }
    delete[] pbuffer;

    if ((*(pbuf + 1) == (uchar)0x38) && (*(pbuf + 2) == (uchar)0x00))
    {
        *(pbuf + 0) = tempBuff[0];
        *(pbuf + 1) = tempBuff[1];
        *(pbuf + 2) = tempBuff[2];

        *(pbuf + 0x37FB) = (uchar)0x00;
        *(pbuf + 0x37FC) = (uchar)0x00;
        *(pbuf + 0x37FD) = (uchar)0x00;
    }

    length = RealLength;

    /*FILE *WF=fopen("2.bin","w+");
    fwrite(pbuf,8,(14*1024),WF);
    fclose(WF);

    printf("write end\n");

    fflush(stdout);*/

    return 2;
}




int MainWindow::downloadProgress()
{
    if (devh!=NULL)
    {
        uchar *reportData = new uchar[6];
        memset(reportData, 0x45, 6);

       // mw.setlabeltxt("1");

        reportData[0] = 0x05;//report id
        int rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);

        if(rc<0)
        {
            QMessageBox::critical(0 ,
            "Error" , "device handle error",
            QMessageBox::Ok,
            0 ,  0 );
           // libusb_close (devh);
           // libusb_exit (ctx);
            delete[] reportData;
            return 2;
        }

        printf("erasure flash\r\n");

        sleep(2);//

        if (binfile != NULL)
        {
            int nn = 0;
            for (nn = 0; nn<5; nn++)
            {
                if (writedata(devh,
                    binfile,
                    2050,
                    binlen))
                    break;

            }

            if (nn >= 5)
            {
                QMessageBox::critical(0 ,
                "Error" , "write fail",
                QMessageBox::Ok,
                0 ,  0 );
                //libusb_close (devh);
                //libusb_exit (ctx);
                delete[] reportData;
                return 2;
            }

            nn = 0;
            for (nn = 0; nn<10; nn++)
            {
                if (ReadBackCheckdata(devh,
                    binfile,
                    2050,
                    binlen))
                    break;
            }

            if (nn >= 10)
            {
                QMessageBox::critical(0 ,
                "Error" , "read fail",
                QMessageBox::Ok,
                0 ,  0 );
               // libusb_close (devh);
               // libusb_exit (ctx);
                delete[] reportData;
                return 2;
            }



            if (!writeSerialNumber(devh,
                6))
            {
                QMessageBox::critical(0 ,
                "Error" , "write SN error",
                QMessageBox::Ok,
                0 ,  0 );
               // libusb_close (devh);
               // libusb_exit (ctx);
                delete[] reportData;
                return 2;
            }

            memset(reportData, 0x55, 6);
            reportData[0] = 0x05;//report id

            rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);

            if(rc<0)
            {
                QMessageBox::critical(0 ,
                "Error" , "device handle error",
                QMessageBox::Ok,
                0 ,  0 );
                //libusb_close (devh);
                //libusb_exit (ctx);
                delete[] reportData;
                return 2;
            }
            else
            {
                //libusb_close (devh);
                //libusb_exit (ctx);
                delete[] reportData;
                return 0;
            }
        }
        else
        {
            //libusb_close (devh);
            //libusb_exit (ctx);
            delete[] reportData;
            return 2;
        }
    }
    else
    {
        return 2;
    }
}


bool MainWindow::writeSerialNumber(libusb_device_handle *handleDev, int cmdlength)
{
    uchar m_sensor_direct = 1;
    uint16_t m_serial_number=0x4100;

    uchar *reportData = new uchar[6];
    //ÔO¶šµØÖ·ºÍéL¶È
    reportData[0] = 0x05;//report id
    reportData[1] = 0x52;
    reportData[2] = 0x80;
    reportData[3] = 0xFF;
    reportData[4] = 0x08;
    reportData[5] = 0x00;
   // BOOL success = HidD_SetFeature(handleDev, reportData, 6);
    int rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);

    if(rc<0)
    {
        //libusb_close (devh);
        //libusb_exit (ctx);
        delete[] reportData;
        return false;
    }

    memset(reportData, 0, 6);
    reportData[0] = 0x05;//report id
    reportData[1] = 0x72;
   // success = HidD_GetFeature(handleDev, reportData, 6);
    rc = libusb_control_transfer(devh, 0xA1, 0x01, 0x0305, 0, reportData, 6, 100);

    if(rc<0)
    {
        //libusb_close (devh);
       // libusb_exit (ctx);
        delete[] reportData;
        return false;
    }

    uint16_t getVid = (reportData[2] << 8) | reportData[3];
    uint16_t getPid = (reportData[4] << 8) | reportData[5];

   // success = HidD_GetFeature(handleDev, reportData, 6);
    rc = libusb_control_transfer(devh, 0xA1, 0x01, 0x0305, 0, reportData, 6, 100);

    if(rc<0)
    {
        //libusb_close (devh);
        //libusb_exit (ctx);
        delete[] reportData;
        return false;
    }

    reportData[0] = 0x05;//report id
    reportData[1] = 0x65;
    reportData[2] = 0xFF;
    reportData[3] = 0x00;
    reportData[4] = 0x00;
    reportData[5] = 0x00;
   // success = HidD_SetFeature(handleDev, reportData, 6);
    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);

    if(rc<0)
    {
        //libusb_close (devh);
       // libusb_exit (ctx);
        delete[] reportData;
        return false;
    }
    usleep(200000);

    reportData[0] = 0x05;//report id
    reportData[1] = 0x57;
    reportData[2] = 0x80;
    reportData[3] = 0xFF;
    reportData[4] = 0x08;
    reportData[5] = 0x00;
    //success = HidD_SetFeature(handleDev, reportData, 6);
    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);

    if(rc<0)
    {
        //libusb_close (devh);
        //libusb_exit (ctx);
        delete[] reportData;
        return false;
    }

    reportData[0] = 0x05;//report id
    reportData[1] = 0x77;
    reportData[2] = (getVid >> 8) & 0xFF;
    reportData[3] = getVid & 0xFF;
    reportData[4] = (getPid >> 8) & 0xFF;
    reportData[5] = getPid & 0xFF;

   // success = HidD_SetFeature(handleDev, reportData, 6);
    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);

    if(rc<0)
    {
        //libusb_close (devh);
        //libusb_exit (ctx);
        delete[] reportData;
        return false;
    }
    usleep(100000);

    reportData[0] = 0x05;//report id
    reportData[1] = 0x77;
    reportData[2] = m_sensor_direct & 0xFF;
    reportData[3] = 0x00;
    reportData[4] = (m_serial_number >> 8) & 0xFF;
    reportData[5] = m_serial_number & 0xFF;

   // success = HidD_SetFeature(handleDev, reportData, 6);
    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);

    if(rc<0)
    {
        //libusb_close (devh);
        //libusb_exit (ctx);
        delete[] reportData;
        return false;
    }

    m_serial_number++;
    delete[] reportData;
   // myProCtrl2->SetPos(pos++);
    return true;
}

bool MainWindow::ReadBackCheckdata(libusb_device_handle *lbdev, uchar *pfBin, int cmdlength, int datalength)
{
    uchar *reportData = new uchar[cmdlength];

    reportData[0] = 0x05;//report id
    reportData[1] = 0x52;
    reportData[2] = 0x00;
    reportData[3] = 0x00;
    reportData[4] = datalength & 0xFF;
    reportData[5] = (datalength >> 8) & 0xFF;
   // BOOL success = HidD_SetFeature(handleShortCmdDev, reportData, cmdlength);
    int rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);
    //posstr = "0%";
    if(rc<0)
    {
       // libusb_close (devh);
        //libusb_exit (ctx);
        delete[] reportData;
        return false;
    }
    else
    {
        uchar *pCmd = new uchar[2050];
        uchar *tempBin = new uchar[2048];
        memset(pCmd, 0, 2050);
        memset(tempBin, 0, 2048);

        pCmd[0] = 0x06;//report id
        pCmd[1] = 0x72;

        uchar buffer[2048];
        for (int i = 0; i < datalength / 2048; i++)
        {

            //success = HidD_GetFeature(handleLongCmdDev, pCmd, 2050);
            rc = libusb_control_transfer(devh, 0xa1, 0x01, 0x0306, 0, pCmd, 2050, 2000);

            if(rc<0)
            {
                //libusb_close (devh);
                //libusb_exit (ctx);
                delete[] reportData;
                delete[] pCmd;
                delete[] tempBin;
                return false;
            }
            else
            {
                usleep(10000);
                float_t pos =(float_t)(i+1)*2048.0/(float_t)datalength;
                pos*=100;
                printf("read data %.2lf\r",pos);
                fflush(stdout);
                printf("\n");
               // QString data = QString("%1").arg(pos);
              //  posstr =data+"%";
                //ui->label_3->clear();
                //ui->label_3->setText(data+"%");
                // emit   Log(QString("FASDFASDFASDF"));
               // Thread.start();
                //setlabeltxt(data);

                //myProCtrl2->SetPos(pos++);

                memset(buffer, 0, 2048);
                memcpy(tempBin, &pfBin[i*2048],2048);
                if (memcmp(tempBin, &pCmd[2], 2048) != 0)
                {
                    //libusb_close (devh);
                    //libusb_exit (ctx);
                    delete[] reportData;
                    delete[] pCmd;
                    delete[] tempBin;
                    return false;
                }
            }

        }
        delete[] tempBin;
        delete[] pCmd;
    }

    delete[] reportData;
    usleep(100000);
    return true;
}

bool MainWindow::writedata(libusb_device_handle *lbdev, uchar *pfBin, int cmdlength, int datalength)
{
    uchar *reportData = new uchar[cmdlength];

    reportData[0] = 0x05;//report id
    reportData[1] = 0x57;
    reportData[2] = 0x00;
    reportData[3] = 0x00;
    reportData[4] = datalength & 0xFF;
    reportData[5] = (datalength >> 8) & 0xFF;
   // bool success = HidD_SetFeature(handleShortCmdDev, reportData, cmdlength);
    int rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);
    //posstr = "0%";
    if(rc<0)
    {
        //libusb_close (devh);
        //libusb_exit (ctx);
        delete[] reportData;
        return false;
    }
    else
    {
        uchar *pCmd = new uchar[2050];

        memset(pCmd, 0, 2050);

        pCmd[0] = 0x06;//report id
        pCmd[1] = 0x77;

        uchar buffer[2048];
        int t = 0;;
        for (int i = 0; i < datalength / 2048; i++)
        {
            memset(buffer, 0, 2048);
            memcpy(buffer, &pfBin[i*2048], 2048);
            if (i == 0)
            {
                buffer[0] = 0x00;
            }
            memcpy(&(pCmd[2]), buffer, 2048);
            //success = HidD_SetFeature(handleLongCmdDev, pCmd, 2050);
            rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0306, 0, pCmd, 2050, 2000);

            if(rc<0)
            {
                //libusb_close (devh);
               // libusb_exit (ctx);
                delete[] reportData;
                delete[] pCmd;
                return false;
            }
            else
            {
                usleep(10000);
                float_t pos =(float_t)(i+1)*2048.0/(float_t)datalength;
                pos*=100;
                QString data = QString("%1").arg(pos);
                printf("write data %.2lf\r",pos);
                fflush(stdout);
                printf("\n");
                //posstr =data+"%";
                //ui->label_3->clear();
               // ui->label_3->setText(data+"%");
               // emit   Log(QString(data+"%"));
                //setlabeltxt(data);

               // myProCtrl2->SetPos(pos++);
            }

            t = i;
        }
        {
           // HidD_SetFeature(handleShortCmdDev, reportData, cmdlength);
            rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, reportData, 6, 100);

            if(rc<0)
            {
                //libusb_close (devh);
                //libusb_exit (ctx);
                delete[] reportData;
                return false;
            }
            else
            {
                memset(buffer, 0, 2048);
                memcpy(buffer, &pfBin[0], 2048);

                memset(pCmd, 0, 2050);
                pCmd[0] = 0x06;//report id
                pCmd[1] = 0x77;
                memcpy(&(pCmd[2]), buffer, 2048);
                //HidD_SetFeature(handleLongCmdDev, pCmd, 2050);
                rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0306, 0, pCmd, 2050, 2000);

                if(rc<0)
                {
                    //libusb_close (devh);
                   // libusb_exit (ctx);
                    delete[] reportData;
                    delete[] pCmd;
                    return false;
                }
            }

        }
        delete[] pCmd;
    }

    delete[] reportData;
    return true;
}



void MainWindow::on_pushButton_2_clicked()
{
emit   Log(QString("FFFFFFFFFFFFFFFFFFFFFFFFFFFF"));
}
