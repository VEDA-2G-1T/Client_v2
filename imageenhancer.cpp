#include "imageenhancer.h"
#include <opencv2/opencv.hpp>
#include <QImage>

QPixmap ImageEnhancer::enhanceSharpness(const QPixmap &pixmap) {
    if (pixmap.isNull()) return pixmap;

    QImage qimg = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(qimg.height(), qimg.width(), CV_8UC3,
                (uchar*)qimg.bits(), qimg.bytesPerLine());

    cv::Mat sharpened;
    cv::Mat kernel = (cv::Mat_<float>(3,3) <<
                          -1, -1, -1,
                      -1,  9, -1,
                      -1, -1, -1);
    cv::filter2D(mat, sharpened, mat.depth(), kernel);

    QImage resultImg((uchar*)sharpened.data,
                     sharpened.cols, sharpened.rows,
                     sharpened.step,
                     QImage::Format_RGB888);
    return QPixmap::fromImage(resultImg.copy());
}

QPixmap ImageEnhancer::enhanceCLAHE(const QPixmap &pixmap) {
    if (pixmap.isNull()) return pixmap;

    QImage qimg = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(qimg.height(), qimg.width(), CV_8UC3,
                (uchar*)qimg.bits(), qimg.bytesPerLine());

    cv::Mat labImg;
    cv::cvtColor(mat, labImg, cv::COLOR_RGB2Lab);

    std::vector<cv::Mat> labPlanes(3);
    cv::split(labImg, labPlanes);

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(labPlanes[0], labPlanes[0]);

    cv::merge(labPlanes, labImg);
    cv::Mat result;
    cv::cvtColor(labImg, result, cv::COLOR_Lab2RGB);

    QImage resultImg((uchar*)result.data,
                     result.cols, result.rows,
                     result.step,
                     QImage::Format_RGB888);
    return QPixmap::fromImage(resultImg.copy());
}
