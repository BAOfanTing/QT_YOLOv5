#include "yolov5.h"
using namespace std;
using namespace cv;

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
    qDebug()<<"模型初始化成功";
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

// YOLOv5目标检测函数
void YOLOv5::detect(cv::Mat &frame)
{
    // 将输入图像转换为神经网络的blob格式，并进行归一化和大小调整
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(this->inpWidth, this->inpHeight), cv::Scalar(0, 0, 0), true);
    // 将blob设置为网络的输入
    this->net.setInput(blob);
    // 运行前向传播，得到网络输出
    this->net.forward(outs, this->net.getUnconnectedOutLayersNames());

    // 清除之前的检测结果
    classIds.clear();
    confidences.clear();
    boxes.clear();
    // 计算从模型输入尺寸到原始图像尺寸的缩放比例
    float ratioh = (float)frame.rows / this->inpHeight, ratiow = (float)frame.cols / this->inpWidth;
    int n = 0, q = 0, i = 0, j = 0, nout = 8 + 5, c = 0;
    for (n = 0; n < 3; n++)   ///尺度
    {
        // 计算特征图的网格数量
        int num_grid_x = (int)(this->inpWidth / this->stride[n]);
        int num_grid_y = (int)(this->inpHeight / this->stride[n]);
        int area = num_grid_x * num_grid_y; // 网格总数
        // 对网络输出进行sigmoid处理
        this->sigmoid(&outs[n], 3 * nout * area);
        for (q = 0; q < 3; q++)    ///anchor数
        {
            // 获取当前尺度下的锚框宽度和高度
            const float anchor_w = this->anchors[n][q * 2];
            const float anchor_h = this->anchors[n][q * 2 + 1];
            float* pdata = (float*)outs[n].data + q * nout * area; // 当前锚框的网络输出
            for (i = 0; i < num_grid_y; i++) // 遍历网格y方向
            {
                for (j = 0; j < num_grid_x; j++) // 遍历网格x方向
                {
                    // 获取当前格子的置信度
                    float box_score = pdata[4 * area + i * num_grid_x + j];
                    if (box_score > this->objThreshold) // 如果置信度大于阈值
                    {
                        float max_class_socre = 0, class_socre = 0;
                        int max_class_id = 0;
                        for (c = 0; c < 80; c++) //// get max socre
                        {
                            // 获取类别置信度
                            class_socre = pdata[(c + 5) * area + i * num_grid_x + j];
                            if (class_socre > max_class_socre)
                            {
                                max_class_socre = class_socre;
                                max_class_id = c; // 记录最大类别置信度及其索引
                            }
                        }

                        if (max_class_socre > this->confThreshold) // 如果类别置信度大于阈值
                        {
                            // 计算检测框的中心坐标、宽度和高度
                            float cx = (pdata[i * num_grid_x + j] * 2.f - 0.5f + j) * this->stride[n];  ///cx
                            float cy = (pdata[area + i * num_grid_x + j] * 2.f - 0.5f + i) * this->stride[n];   ///cy
                            float w = powf(pdata[2 * area + i * num_grid_x + j] * 2.f, 2.f) * anchor_w;   ///w
                            float h = powf(pdata[3 * area + i * num_grid_x + j] * 2.f, 2.f) * anchor_h;  ///h

                            // 将检测框的坐标还原到原图上
                            int left = (cx - 0.5*w)*ratiow;
                            int top = (cy - 0.5*h)*ratioh;   ///坐标还原到原图上

                            // 将检测结果保存到相应的容器中
                            classIds.push_back(max_class_id);
                            confidences.push_back(max_class_socre);
                            boxes.push_back(Rect(left, top, (int)(w*ratiow), (int)(h*ratioh)));
                        }
                    }
                }
            }
        }
    }

    indices.clear();
    cv::dnn::NMSBoxes(boxes, confidences, this->confThreshold, this->nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        // 绘制预测框及其类别和置信度
        this->drawPred(classIds[idx], confidences[idx], box.x, box.y,
                       box.x + box.width, box.y + box.height, frame);
    }
}

void YOLOv5::drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat &frame)
{
    // 绘制检测框
    rectangle(frame, Point(left, top), Point(right, bottom), Scalar(0, 0, 255), 3);

    // 构建标签，包含类别名称和置信度
    string label = format("%.2f", conf);
    label = this->classes[classId] + ":" + label;

    int baseLine;
    // 获取标签尺寸
    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);
    // 绘制标签
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 1);
}

void YOLOv5::sigmoid(cv::Mat *out, int length)
{
    float* pdata = (float*)(out->data);
    int i = 0;
    // 对网络输出进行sigmoid处理
    for (i = 0; i < length; i++)
    {
        pdata[i] = 1.0 / (1 + expf(-pdata[i]));
    }
}

