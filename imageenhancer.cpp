#include "imageenhancer.h"
#include <opencv2/opencv.hpp>

QImage ImageEnhancer::enhance(const QByteArray &imageData)
{
    std::vector<uchar> data(imageData.begin(), imageData.end());
    cv::Mat img = cv::imdecode(data, cv::IMREAD_COLOR);

    if (img.empty()) return QImage();

    // 🔍 선명도 향상 - Unsharp Mask
    cv::Mat blurred, sharp;
    cv::GaussianBlur(img, blurred, cv::Size(0, 0), 3);
    cv::addWeighted(img, 1.5, blurred, -0.5, 0, sharp);

    // BGR → RGB
    cv::cvtColor(sharp, sharp, cv::COLOR_BGR2RGB);

    return QImage(sharp.data, sharp.cols, sharp.rows, sharp.step, QImage::Format_RGB888).copy();
}
