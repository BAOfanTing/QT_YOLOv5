#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDebug>
#include <opencv.hpp>
#include <QTimer>
#include "yolov5.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_openfile_clicked();

    //播放视频
    void updateFrame();

    void on_btn_startdetect_clicked();

    void on_btn_camera_clicked();


    void on_btn_loadmodel_clicked();

    void on_btn_stopdetect_clicked();

private:
    Ui::MainWindow *ui;

    cv::VideoCapture *capture;

    //判断读取的文件类型
    QString filetype = "";

    //播放视频和启动摄像头的定时器
    QTimer *timer;

    //定义yolov5指针
    YOLOv5 *yolov5;

    NetConfig conf;
    NetConfig *yolo_nets;
    std::vector<cv::Rect> bboxes;

    bool canDetect = false;
    bool is_loadedmodel = false;
};
#endif // MAINWINDOW_H
