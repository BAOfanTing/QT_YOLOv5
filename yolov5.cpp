#include "yolov5.h"

YOLOv5::YOLOv5() {}

// YOLOv5对象的初始化函数
void YOLOv5::Init(NetConfig config)
{
    // 设置类别置信度阈值
    this->confThreshold = config.confThreshold;
    // 设置非最大抑制阈值
    this->nmsThreshold = config.nmsThreshold;
    // 设置目标置信度阈值
    this->objThreshold = config.objThreshold;

    // 预分配内存以优化性能，以下是基于经验的估计值
    classIds.reserve(20);     // 预留空间用于存储检测到的类别ID
    confidences.reserve(20);  // 预留空间用于存储检测到的类别的置信度
    boxes.reserve(20);        // 预留空间用于存储检测到的边界框
    outs.reserve(3);          // 预留空间用于存储网络的输出
    indices.reserve(20);      // 预留空间用于存储非最大抑制后保留的检测框的索引
}

// 加载ONNX模型文件
bool YOLOv5::loadModel(QString onnxfile)
{
    try
    {
        // 使用OpenCV从ONNX文件读取网络模型,不接受QString
        this->net = cv::dnn::readNetFromONNX(onnxfile.toStdString());

        // 检查是否有可用的CUDA设备（即检查是否可以使用GPU进行加速）
        int deviceID = cv::cuda::getCudaEnabledDeviceCount();
        if(deviceID == 1)
        {
            // 如果有可用的CUDA设备，将网络的推理后端设置为CUDA以使用GPU
            this->net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
            this->net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
        }
        else
        {
            // 如果没有检测到CUDA设备，则弹出消息框提示用户当前使用CPU进行推理
            QMessageBox::information(NULL, "warning", QStringLiteral("正在使用CPU推理！\n"), QMessageBox::Yes, QMessageBox::Yes);
        }
        return true; // 模型加载成功，返回true
    }
    catch(std::exception& e)
    {
        // 如果在加载模型过程中发生异常，弹出消息框提示错误信息，并返回false
        QMessageBox::critical(NULL, "Error", QStringLiteral("模型加载出错，请检查重试！\n %1").arg(e.what()), QMessageBox::Yes, QMessageBox::Yes);
        return false;
    }
}
