#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QQueue>
#include <QKeyEvent>
namespace Ui {
class client;
}

class client : public QMainWindow
{
    Q_OBJECT

public:
    explicit client(QWidget *parent = 0);
    ~client();
    QImage show_photo(QString addr,uint32_t threshod);
    void process_data(QByteArray data);
private slots:
    void on_pushButton_connect_clicked();

    void on_pushButton_close_clicked();


    void on_pushButton_front_pressed();

    void on_pushButton_front_released();

    void on_pushButton_back_pressed();

    void on_pushButton_back_released();

    void on_pushButton_left_pressed();

    void on_pushButton_left_released();

    void on_pushButton_right_pressed();

    void on_pushButton_right_released();

    void on_pushButton_turn_left_pressed();

    void on_pushButton_turn_left_released();

    void on_pushButton_turn_right_pressed();

    void on_pushButton_turn_right_released();
    void timer1_send_task();
    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();

    void on_pushButton_config_clicked();

private:
    Ui::client *ui;
    QTimer* timer1;
    QQueue<uint8_t> q;//缓存队列
    void unpack_data(uint8_t* buff,uint8_t size);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
};


/*__attribute((packed))*/  // 取消结构在编译过程中的优化对齐，按照实际占用字节数进行对齐。
#pragma pack(push) //保存对齐状态
#pragma pack(1)   // 1 bytes对齐
typedef struct
{
  uint8_t head1;  //帧头
  uint8_t head2;  //帧头
  uint8_t size;  //本帧大小
  uint8_t cmd;   //本帧类型
}frame_head_t;

typedef struct
{
  frame_head_t frame_head;

  uint8_t key;  //控制按钮
  uint8_t mode; //模式

}frame_normol_t;  //通常帧

typedef struct
{
  frame_head_t frame_head;

  float servo[12]; //舵机角度

}frame_servo_t;  //通常帧


typedef struct
{
 frame_head_t frame_head;
 float yaw;
 float pitch;
 float roll;
 float vx;
 float vy;
 float vz;
 float ax;
 float ay;
 float az;
}frame_mpu6050_t;  //通常帧

#pragma pack(pop)//恢复对齐状态


#endif // CLIENT_H
