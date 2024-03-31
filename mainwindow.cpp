#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 设置QLabel的对齐方式为居中
    ui->lb_show->setAlignment(Qt::AlignCenter);
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
        // 使用OpenCV读取图片
        cv::Mat src = cv::imread(filename.toStdString());
        // 检查图片是否正确读取，即图片数据是否非空
        if(src.empty())
        {
            ui->lb_show->setText("图像不存在"); // 如果图片不存在，显示错误消息
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
        // 将OpenCV的Mat数据转换为QImage对象
        QImage img = QImage(temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
        // 将QImage对象转换为QPixmap对象，并根据标签的高度调整图片大小
        QPixmap mmp = QPixmap::fromImage(img);
        mmp = mmp.scaledToHeight(ui->lb_show->height());
        // 将调整后的图片显示在标签上
        ui->lb_show->setPixmap(mmp);
    }
    else if(mime.name().startsWith("video/"))
    {

    }

}


