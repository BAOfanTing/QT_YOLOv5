#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDebug>
#include <opencv.hpp>
#include <QTimer>

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

private:
    Ui::MainWindow *ui;

    cv::VideoCapture *capture;

    //判断读取的文件类型
    QString filetype = "";

    //播放视频定时器
    QTimer *timer;
};
#endif // MAINWINDOW_H
