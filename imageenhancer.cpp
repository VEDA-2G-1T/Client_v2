#include "imageenhancer.h"
#include <opencv2/opencv.hpp>
#include <QImage>

// ✅ CLAHE 대비 향상
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

// ✅ 슬라이더 기반 가변 샤프닝
QPixmap ImageEnhancer::enhanceSharpness(const QPixmap &pixmap, int level) {
    if (pixmap.isNull()) return pixmap;

    QImage qimg = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(qimg.height(), qimg.width(), CV_8UC3,
                (uchar*)qimg.bits(), qimg.bytesPerLine());

    // level: 0~100 → 강도 매핑 (1.0 ~ 5.0)
    float alpha = 1.0f + (level / 25.0f);

    cv::Mat blurred;
    cv::GaussianBlur(mat, blurred, cv::Size(0,0), 3);

    cv::Mat sharp;
    cv::addWeighted(mat, alpha, blurred, -(alpha - 1.0f), 0, sharp);

    QImage resultImg((uchar*)sharp.data,
                     sharp.cols, sharp.rows,
                     sharp.step,
                     QImage::Format_RGB888);
    return QPixmap::fromImage(resultImg.copy());
}
