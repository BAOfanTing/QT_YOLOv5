#ifndef YOLOV5_H
#define YOLOV5_H
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/cuda.hpp>
#include <QMessageBox>


struct NetConfig
{
    float confThreshold; // 类别置信度阈值
    float nmsThreshold;  // 非最大抑制（NMS）阈值
    float objThreshold;  // 目标置信度阈值
    std::string netname; // 网络名称
};



class YOLOv5
{
public:
    YOLOv5();
    void Init(NetConfig config);
    bool loadModel(const char* onnxfile);
    void detect(cv::Mat& frame);

private:
    float confThreshold; // 类别置信度阈值
    float nmsThreshold;  // 非最大抑制（NMS）阈值
    float objThreshold;  // 目标置信度阈值
    // 定义锚点尺寸，每个尺寸组对应不同尺度的特征图
    const float anchors[3][6] = {
        {10.0, 13.0, 16.0, 30.0, 33.0, 23.0},
        {30.0, 61.0, 62.0, 45.0, 59.0, 119.0},
        {116.0, 90.0, 156.0, 198.0, 373.0, 326.0}
    };
    // 定义特征图的步长
    const float stride[3] = {8.0, 16.0, 32.0};
    // 定义可能检测到的类别
    std::string classes[80] = {
        "person", "bicycle", "car", "motorbike", "aeroplane", "bus",
        "train", "truck", "boat", "traffic light", "fire hydrant",
        "stop sign", "parking meter", "bench", "bird", "cat", "dog",
        "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
        "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat",
        "baseball glove", "skateboard", "surfboard", "tennis racket",
        "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
        "banana", "apple", "sandwich", "orange", "broccoli", "carrot",
        "hot dog", "pizza", "donut", "cake", "chair", "sofa", "pottedplant",
        "bed", "diningtable", "toilet", "tvmonitor", "laptop", "mouse",
        "remote", "keyboard", "cell phone", "microwave", "oven", "toaster",
        "sink", "refrigerator", "book", "clock", "vase", "scissors",
        "teddy bear", "hair drier", "toothbrush"
    };
    // 输入图像的宽和高
    const int inpWidth = 640;
    const int inpHeight = 640;

    // 用于存储网络输出的向量
    std::vector<cv::Mat> outs;
    // 检测到的类别ID
    std::vector<int> classIds;
    // 检测到的类别的置信度
    std::vector<float> confidences;
    // 检测到的对象的边界框
    std::vector<cv::Rect> boxes;
    // 用于非最大抑制后保留的检测框的索引
    std::vector<int> indices;
    // 神经网络
    cv::dnn::Net net;
};

#endif // YOLOV5_H
