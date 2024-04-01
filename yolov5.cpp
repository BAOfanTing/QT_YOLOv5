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


