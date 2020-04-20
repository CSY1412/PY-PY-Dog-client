#include "client.h"
#include "ui_client.h"
#include "QMessageBox"
#include "QImage"
#include "QPixmap"
#include "QtMath"


QTcpSocket *TcpSocket;  //全局变量
frame_normol_t frame_normol; //通常帧
frame_servo_t frame_servo;   //舵机角度帧
frame_mpu6050_t frame_mpu6050; //陀螺仪帧

#define FRAME_HEAD1 0x5A
#define FRAME_HEAD2 0x5B

/**
  * @brief     client构造函数
  * @author   SiYu Chen
  * @param[in]
  * @retval
  */
client::client(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::client)
{
    ui->setupUi(this);

    this->setWindowTitle("PY-PY DOG客户端v1.1");  //设置标题
    ui->label_logo->setPixmap(QPixmap::fromImage(client::show_photo(QString(":/new/prefix1/logo.png")
                                ,0xFFFB0000))); //显示菠萝狗logo
    ui->label_logo_2->setPixmap(QPixmap::fromImage(client::show_photo(QString(":/new/prefix1/logo2.png")
                                ,0xFF800000))); //显示菠萝狗logo

    ui->lineEdit_port->setMaximumWidth(50); //设置端口宽度
    ui->lineEdit_ip->setMaximumWidth(120);//设置IP宽度
    ui->statusBar->addWidget(ui->label);   //往状态栏加东西
    ui->statusBar->addWidget(ui->lineEdit_ip);  //加入IP
    ui->statusBar->addWidget(ui->label_2);
    ui->statusBar->addWidget(ui->lineEdit_port);  //加入端口输入
    ui->statusBar->addWidget(ui->pushButton_connect); //加入开始连接按钮
    ui->statusBar->addWidget(ui->pushButton_close); //加入断开连接按钮


    ui->statusBar->addPermanentWidget(ui->label_4); //显示连接状态
    ui->statusBar->addPermanentWidget(ui->label_connect_state);//从右边添加

    ui->pushButton_close->setEnabled(false);  //使能无法点击
    ui->pushButton_config->setEnabled(false);   //

    TcpSocket=new QTcpSocket(this);  //创立一个tcpsocket
    timer1 =new QTimer(this);   //创建一个定时器


    connect(TcpSocket,&QTcpSocket::connected,[=]()  //链接成功的响应
    {
        ui->label_connect_state->setText("已连接");
        timer1->start(20);  //打开发送数据任务
        ui->pushButton_close->setEnabled(true);
        ui->pushButton_connect->setEnabled(false);

    });
    connect(TcpSocket,&QTcpSocket::disconnected,[=]()  //断开链接
    {
        ui->label_connect_state->setText("未连接");

    });
    connect(TcpSocket,&QTcpSocket::readyRead,[=]()  //有数据
    {
        QByteArray data=TcpSocket->readAll();
        process_data(data);
    });
    //帮助功能
    connect(ui->actions_about,&QAction::triggered,[=]()
    {
        QMessageBox::about(this,"关于","希望菠萝狗狗越做越完善！！\n"
                                     "v1.1更新说明\n"
                                     "修改原帧头为0x5A 0x5B"
                                 "   ");
    });
    connect(ui->actions_use_method,&QAction::triggered,[=]()
    {
        QMessageBox::about(this,"帮助","可使用按键进行控制"
                                 "w--向前  s--向后  a--向左  d--向右   "
                                  "q--左旋  e--右旋");
    });
    connect(timer1,&QTimer::timeout,[=]()
    {
        //使能关闭设置按钮
        if(ui->radioButton_4->isChecked()) ui->pushButton_config->setEnabled(true);
        else  ui->pushButton_config->setEnabled(false);

        timer1_send_task(); //定时发送任务
    });


}

client::~client()
{
    delete ui;
}


/**
  * @brief    处理来的数据  找到帧头后解包
  * @author   SiYu Chen
  * @param[in]
  * @retval
  */
void client::process_data(QByteArray data)
{
    static bool find_head=0;
    static uint8_t size=0;
    uint8_t buff[80];
    for(int i=0;i<data.length();i++)  //全部插进队列
    {

        q.enqueue(data.at(i));
    }
    while(q.at(0)!=FRAME_HEAD1||q.at(1)!=FRAME_HEAD2) //找帧头
    {
        if(!q.isEmpty())
        {
           q.dequeue();   //去除
           if(q.isEmpty())
           {
               qDebug()<<"这一帧没帧头";
               return;
           }
        }
        else
            return;
    }
    if(q.at(0)==FRAME_HEAD1&&q.at(1)==FRAME_HEAD2)
    {
        if(q.length()>=3)//有第三帧
        {
            size=q.at(2);  //获取尺寸
        }

        if(q.length()>=size&&size!=0)
        {
            for(int i=0;i<size;i++)
            {
                buff[i]=q.dequeue();
            }
           // qDebug()<<"找到帧头";
           //  qDebug()<<"ass";
            unpack_data(buff,size);
        }
    }


}

/**
  * @brief      解包数据
  * @author   SiYu Chen
  * @param[in]
  * @retval
  */
void client::unpack_data(uint8_t* buff,uint8_t size)
{
    qDebug()<<buff[0];
    if(buff[3]==0x24&&ui->radioButton_4->isChecked()==false)  //当是舵机数据的时候
    {
       memcpy(&frame_servo,buff,sizeof(frame_servo));

      ui->doubleSpinBox->setValue( (double) frame_servo.servo[0]);
      ui->doubleSpinBox_2->setValue((double) frame_servo.servo[1]);
      ui->doubleSpinBox_3->setValue((double) frame_servo.servo[2]);

      ui->doubleSpinBox_10->setValue((double)frame_servo.servo[3]);
      ui->doubleSpinBox_11->setValue((double) frame_servo.servo[4]);
      ui->doubleSpinBox_12->setValue((double) frame_servo.servo[5]);

      ui->doubleSpinBox_13->setValue((double)  frame_servo.servo[6]);
      ui->doubleSpinBox_14->setValue((double)  frame_servo.servo[7]);
      ui->doubleSpinBox_15->setValue((double) frame_servo.servo[8]);

      ui->doubleSpinBox_16->setValue((double) frame_servo.servo[9]);
      ui->doubleSpinBox_17->setValue((double) frame_servo.servo[10]);
      ui->doubleSpinBox_18->setValue((double)frame_servo.servo[11]);
    }

    if(buff[3]==0x25)  //当时陀螺仪数据时
    {
       memcpy(&frame_mpu6050,buff,sizeof(frame_mpu6050));
       ui->label_17->setText(QString("%1").arg(floor(frame_mpu6050.yaw*100)/100));
       ui->label_18->setText(QString("%1").arg(floor(frame_mpu6050.pitch*100)/100));
       ui->label_19->setText(QString("%1").arg(floor(frame_mpu6050.roll*100)/100));
       ui->label_20->setText(QString("%1").arg(floor(frame_mpu6050.vx*100)/100));
       ui->label_21->setText(QString("%1").arg(floor(frame_mpu6050.vy*100)/100));
       ui->label_22->setText(QString("%1").arg(floor(frame_mpu6050.vz*100)/100));
       ui->label_23->setText(QString("%1").arg(floor(frame_mpu6050.ax*100)/100));
       ui->label_24->setText(QString("%1").arg(floor(frame_mpu6050.ay*100)/100));
       ui->label_25->setText(QString("%1").arg(floor(frame_mpu6050.az*100)/100));

    }


}
/**
  * @brief    定时发送常规帧  被定时器中断调用
  * @author   SiYu Chen
  * @param[in]
  * @retval
  */
void client::timer1_send_task()
{

     frame_normol.frame_head.head1=FRAME_HEAD1;
     frame_normol.frame_head.head2=FRAME_HEAD2;
     frame_normol.frame_head.cmd=0x21;   //定时帧
     frame_normol.frame_head.size=6;//6字节
    //检测界面中的模式
    if(ui->radioButton->isChecked()) frame_normol.mode=0x00;
    if(ui->radioButton_2->isChecked()) frame_normol.mode=0x01;
    if(ui->radioButton_3->isChecked()) frame_normol.mode=0x02;
    if(ui->radioButton_4->isChecked()) frame_normol.mode=0x03;


    char buff[20]; //buff

    buff[0]= frame_normol.frame_head.head1;
    buff[1]= frame_normol.frame_head.head2;
    buff[2]= frame_normol.frame_head.size;
    buff[3]= frame_normol.frame_head.cmd;
    buff[4]=frame_normol.key;
    buff[5]=frame_normol.mode;

    TcpSocket->write(buff,frame_normol.frame_head.size); //发送一帧定时帧


}






void client::on_pushButton_connect_clicked()
{
    QString addr=ui->lineEdit_ip->text();
    int16_t port=ui->lineEdit_port->text().toInt();
    TcpSocket->connectToHost(QHostAddress(addr),port);

}

void client::on_pushButton_close_clicked()
{
    TcpSocket->disconnectFromHost();
    TcpSocket->close();
      timer1->stop();
    ui->label_connect_state->setText("未连接"); //显示未连接
    ui->pushButton_close->setEnabled(false);  //使能断开
    ui->pushButton_connect->setEnabled(true); //使能连接
}


/**
  * @brief    处理logo
  * @author   SiYu Chen
  * @param[in]
  * @retval
  */

//0xFFFB0000
QImage client::show_photo(QString addr,uint32_t threshod)
{
     QImage image (addr);
     image = image.convertToFormat(QImage::Format_ARGB32);
     union myrgb
     {
         uint rgba;
         uchar rgba_bits[4];
     };
     myrgb* mybits =(myrgb*) image.bits();
     int len = image.width()*image.height();
     while(len --> 0)
     {
         mybits->rgba_bits[3] = (mybits->rgba>= threshod)?0:255;
         mybits++;
     }

    return image;
}



void client::on_pushButton_front_pressed()
{
    frame_normol.key|=0x01;
}

void client::on_pushButton_front_released()
{
    frame_normol.key&=0xFF-0x01;
}

void client::on_pushButton_back_pressed()
{
    frame_normol.key|=0x02;
}

void client::on_pushButton_back_released()
{
    frame_normol.key&=0xFF-0x02;
}

void client::on_pushButton_left_pressed()
{
     frame_normol.key|=0x04;
}

void client::on_pushButton_left_released()
{
     frame_normol.key&=0xFF-0x04;
}

void client::on_pushButton_right_pressed()
{
    frame_normol.key|=0x08;
}

void client::on_pushButton_right_released()
{
    frame_normol.key&=0xFF-0x08;
}

void client::on_pushButton_turn_left_pressed()
{
    frame_normol.key|=0x10;
}

void client::on_pushButton_turn_left_released()
{
    frame_normol.key&=0xFF-0x10;
}

void client::on_pushButton_turn_right_pressed()
{
    frame_normol.key|=0x20;
}

void client::on_pushButton_turn_right_released()
{
    frame_normol.key&=0xFF-0x20;
}

void client::on_pushButton_start_clicked()
{
    frame_normol.frame_head.head1=FRAME_HEAD1;
     frame_normol.frame_head.head2=FRAME_HEAD2;
     frame_normol.frame_head.cmd=0x22;   //start命令
     frame_normol.frame_head.size=4;//3个字节

    char buff[20]; //buff

    buff[0]= frame_normol.frame_head.head1;
    buff[1]= frame_normol.frame_head.head2;
    buff[2]= frame_normol.frame_head.size;
    buff[3]= frame_normol.frame_head.cmd;

    TcpSocket->write(buff, frame_normol.frame_head.size); //发送一帧突发帧
}

void client::on_pushButton_stop_clicked()
{
    frame_normol.frame_head.head1=FRAME_HEAD1;
    frame_normol.frame_head.head2=FRAME_HEAD2;
    frame_normol.frame_head.cmd=0x23;   //stop命令
    frame_normol.frame_head.size=4;//3个字节

    char buff[20]; //buff

    buff[0]=frame_normol.frame_head.head1;
    buff[1]=frame_normol.frame_head.head2;
    buff[2]=frame_normol.frame_head.size;
    buff[3]=frame_normol.frame_head.cmd;

    TcpSocket->write(buff,frame_normol.frame_head.size); //发送一帧突发帧
}

void client::on_pushButton_config_clicked()
{
    if(ui->radioButton_4->isChecked())  //当在调试模式的时候
    {
        frame_servo.frame_head.head1=FRAME_HEAD1;
        frame_servo.frame_head.head2=FRAME_HEAD2;
        frame_servo.frame_head.cmd=0x24;   //舵机值
        frame_servo.frame_head.size=sizeof (frame_servo);

        frame_servo.servo[0]=ui->doubleSpinBox->value();
        frame_servo.servo[1]=ui->doubleSpinBox_2->value();
        frame_servo.servo[2]=ui->doubleSpinBox_3->value();

        frame_servo.servo[3]=ui->doubleSpinBox_10->value();
        frame_servo.servo[4]=ui->doubleSpinBox_11->value();
        frame_servo.servo[5]=ui->doubleSpinBox_12->value();

        frame_servo.servo[6]=ui->doubleSpinBox_13->value();
        frame_servo.servo[7]=ui->doubleSpinBox_14->value();
        frame_servo.servo[8]=ui->doubleSpinBox_15->value();

        frame_servo.servo[9]=ui->doubleSpinBox_16->value();
        frame_servo.servo[10]=ui->doubleSpinBox_17->value();
        frame_servo.servo[11]=ui->doubleSpinBox_18->value();


        TcpSocket->write((char*)&frame_servo,frame_servo.frame_head.size); //发送一帧突发帧
    }
}

void client::keyPressEvent(QKeyEvent *e)
{

    switch(e->key())
    {
        case Qt::Key_W : frame_normol.key|=0x01; break;
        case Qt::Key_S :frame_normol.key|=0x02; break;
        case Qt::Key_A :frame_normol.key|=0x04; break;
        case Qt::Key_D : frame_normol.key|=0x08; break;
        case Qt::Key_Q : frame_normol.key|=0x10; break;
        case Qt::Key_E :frame_normol.key|=0x20; break;
        default : break;
    }

}
void client::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_W : frame_normol.key&=0xFF-0x01; break;
        case Qt::Key_S : frame_normol.key&=0xFF-0x02;  break;
        case Qt::Key_A :frame_normol.key&=0xFF-0x04;  break;
        case Qt::Key_D :frame_normol.key&=0xFF-0x08; break;
        case Qt::Key_Q : frame_normol.key&=0xFF-0x10; break;
        case Qt::Key_E :frame_normol.key&=0xFF-0x20;  break;
        default : break;
    }

}
