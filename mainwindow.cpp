#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 设置QLabel的对齐方式为居中
    ui->lb_show->setAlignment(Qt::AlignCenter);

    capture = new cv::VideoCapture;
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&MainWindow::updateFrame);
    yolo_nets = new NetConfig[4]{
        {0.5, 0.5, 0.5, "yolov5s"},
        {0.6, 0.6, 0.6, "yolov5m"},
        {0.65, 0.65, 0.65, "yolov5l"},
        {0.75, 0.75, 0.75, "yolov5x"}
    };
    conf = yolo_nets[0];
    yolov5 = new YOLOv5();
    yolov5->Init(conf);
    ui->te_message->append(QStringLiteral("默认模型类别：yolov5s args: %1 %2 %3")
                                .arg(conf.nmsThreshold)
                                .arg(conf.objThreshold)
                                .arg(conf.confThreshold));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btn_openfile_clicked()
{
    // 使用文件系统弹出对话框获得用户选择的文件路径
    QString filename = QFileDialog::getOpenFileName(this, QStringLiteral("打开文件"), ".", "*.mp4 *.avi *.png *.jpg *.jpeg *.bmp");
    // 检查文件是否存在
    if(!QFile::exists(filename))
    {
        return; // 如果文件不存在，则直接返回
    }
    // 在文本编辑框中显示选中的文件路径
    ui->te_message->setText(filename);

    // 使用QMimeDatabase判断文件类型
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filename);
    qDebug() << mime.name(); // 在调试输出中显示文件的MIME类型

    // 如果文件是图片类型
    if(mime.name().startsWith("image/"))
    {
        filetype = "pic";
        // 使用OpenCV读取图片
        cv::Mat src = cv::imread(filename.toStdString());
        // 检查图片是否正确读取，即图片数据是否非空
        if(src.empty())
        {
            ui->te_message->append("图像不存在"); // 如果图片不存在，显示错误消息
            return;
        }
        cv::Mat temp; // 创建一个临时Mat对象用于存放转换后的图片数据

        // 根据图片的通道数进行相应的颜色空间转换
        if(src.channels() == 4)
        {
            // 如果图片是4通道的（如带透明度的PNG），则转换为RGB
            cv::cvtColor(src, temp, cv::COLOR_BGRA2RGB);
        }
        else if(src.channels() == 3)
        {
            // 如果图片是3通道的（如JPG），则转换为RGB
            cv::cvtColor(src, temp, cv::COLOR_BGR2RGB);
        }
        else
        {
            // 如果图片是2通道的或其他情况，这里的注释应该为处理单通道灰度图，将其转换为RGB
            cv::cvtColor(src, temp, cv::COLOR_GRAY2RGB);
        }

        yolov5->detect(temp);
        // 将OpenCV的Mat数据转换为QImage对象
        QImage img = QImage(temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
        // 将QImage对象转换为QPixmap对象，并根据标签的高度调整图片大小
        QPixmap mmp = QPixmap::fromImage(img);
        mmp = mmp.scaledToHeight(ui->lb_show->height());
        // 将调整后的图片显示在标签上
        ui->lb_show->setPixmap(mmp);

        filename.clear();
    }
    else if(mime.name().startsWith("video/"))
    {
        // 设置文件类型为视频
        filetype = "video";
        // 使用OpenCV打开视频文件
        capture->open(filename.toStdString());
        // 如果无法打开视频文件，则在文本编辑区显示错误消息并返回
        if(!capture->isOpened())
        {
            ui->te_message->append("mp4文件打开失败！");
            return;
        }

        // 获取视频的总帧数,宽度，高度，显示信息
        long totalFrame = capture->get(cv::CAP_PROP_FRAME_COUNT);
        int width = capture->get(cv::CAP_PROP_FRAME_WIDTH);
        int height = capture->get(cv::CAP_PROP_FRAME_HEIGHT);
        ui->te_message->append(QString("整个视频共 %1 帧, 宽=%2 高=%3 ").arg(totalFrame).arg(width).arg(height));

        // 设置要从视频的开始帧读取
        long frameToStart = 0;
        capture->set(cv::CAP_PROP_POS_FRAMES, frameToStart);
        ui->te_message->append(QString("从第 %1 帧开始读").arg(frameToStart));

        // 获取视频的帧率
        double frameRate = capture->get(cv::CAP_PROP_FPS);
        ui->te_message->append(QString("帧率为: %1 ").arg(frameRate));

        // 读取视频的第一帧
        cv::Mat frame;
        capture->read(frame);
        // 将帧的颜色空间从BGR转换为RGB
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        // 将OpenCV的Mat对象转换为QImage对象
        QImage videoimg = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        // 将QImage对象转换为QPixmap对象
        QPixmap mmp = QPixmap::fromImage(videoimg);
        // 将QPixmap对象按照标签的高度进行缩放
        mmp = mmp.scaledToHeight(ui->lb_show->height());
        // 在标签上显示这个QPixmap对象
        ui->lb_show->setPixmap(mmp);
    }
}

void MainWindow::updateFrame()
{
    if(filetype == "video")
    {
        cv::Mat frame;
        if(capture->read(frame))
        {
            //读取帧成功
            cv::cvtColor(frame,frame,cv::COLOR_BGR2RGB);
            if(canDetect) yolov5->detect(frame);

            QImage videoimg = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            QPixmap mmp = QPixmap::fromImage(videoimg);
            mmp = mmp.scaledToHeight(ui->lb_show->height());
            ui->lb_show->setPixmap(mmp);
        }
        else
        {
            timer->stop();
        }
    }
    else if(filetype == "camera")
    {
        cv::Mat src;
        if(capture->isOpened())
        {
            //将摄像头数据放入src
            *capture >> src;
            if(src.data == nullptr) return; // 如果图像数据为空，则返回
        }

        //将图像转换为qt能够处理的格式
        cv::Mat frame;
        cv::cvtColor(src,frame,cv::COLOR_BGR2RGB);
        cv::flip(frame,frame,1);//水平翻转图像

        if(canDetect) yolov5->detect(frame);
        QImage videoimg = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        QPixmap mmp = QPixmap::fromImage(videoimg);
        mmp = mmp.scaledToHeight(ui->lb_show->height());  //设置图像的缩放比例
        ui->lb_show->setPixmap(mmp);
    }

}

void MainWindow::on_btn_startdetect_clicked()
{
    //开始检测时封锁其他按钮
    ui->btn_startdetect->setEnabled(false);
    ui->btn_stopdetect->setEnabled(true);
    ui->btn_openfile->setEnabled(false);
    ui->btn_loadmodel->setEnabled(false);
    ui->btn_camera->setEnabled(false);
    ui->comboBox->setEnabled(false);
    ui->te_message->append(QStringLiteral("=======================\n"
                                           "        开始检测\n"
                                           "=======================\n"));
    if(filetype == "pic")
    {
        //对图像进行识别
    }
    else if(filetype == "video")
    {
        //对视频进行识别
        canDetect = true;
        double frameRate = capture->get(cv::CAP_PROP_FPS);
        timer->start(1000/frameRate); // 根据帧率开始播放

    }
    else
    {
        canDetect = true;
        //对摄像头进行识别
    }

}


void MainWindow::on_btn_camera_clicked()
{
    filetype = "camera";
    if(ui->btn_camera->text() == "打开摄像头")
    {
        ui->btn_camera->setText("关闭摄像头");
        //打开摄像头和定时器
        capture->open(0);
        timer->start(100);
    }
    else
    {
        ui->btn_camera->setText("打开摄像头");
        //关闭摄像头和定时器
        capture->release();
        timer->stop();
        ui->lb_show->clear();
    }
}


void MainWindow::on_btn_loadmodel_clicked()
{
    // 使用文件系统弹出对话框获得用户选择的文件路径
    QString onnxfile = QFileDialog::getOpenFileName(this, QStringLiteral("选择模型"), ".", "*.onnx");
    // 检查文件是否存在
    if(!QFile::exists(onnxfile))
    {
        return; // 如果文件不存在，则直接返回
    }
    // 在文本编辑框中显示选中的文件路径
    ui->te_message->setText(onnxfile);
    if(!yolov5->loadModel(onnxfile))
    {
        ui->te_message->append("加载模型失败！");
        return;
    }
    else
    {
         ui->te_message->append("加载模型成功！");
    }
}

