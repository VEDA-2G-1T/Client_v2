#include "imageenhancer.h"
#include <opencv2/opencv.hpp>
#include <QImage>

// ✅ 샤프닝 (-100 ~ +100)
QPixmap ImageEnhancer::enhanceSharpness(const QPixmap &pixmap, int level) {
    if (pixmap.isNull()) return pixmap;

    QImage qimg = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(qimg.height(), qimg.width(), CV_8UC3,
                (uchar*)qimg.bits(), qimg.bytesPerLine());

    if (level == 0) {
        return pixmap; // 원본
    }

    cv::Mat result;
    if (level > 0) {
        // 양수 → 선명도 강화
        float alpha = 1.0f + (level / 50.0f); // 예: +50이면 2.0배
        cv::Mat blurred;
        cv::GaussianBlur(mat, blurred, cv::Size(0,0), 3);
        cv::addWeighted(mat, alpha, blurred, -(alpha - 1.0f), 0, result);
    } else {
        // 음수 → 블러 효과
        int ksize = std::max(1, -level / 10 * 2 + 1); // level=-100일 때 큰 커널
        cv::GaussianBlur(mat, result, cv::Size(ksize, ksize), 0);
    }

    QImage resultImg((uchar*)result.data,
                     result.cols, result.rows,
                     result.step,
                     QImage::Format_RGB888);
    return QPixmap::fromImage(resultImg.copy());
}

// ✅ 대비 (-100 ~ +100)
QPixmap ImageEnhancer::enhanceCLAHE(const QPixmap &pixmap, int level) {
    if (pixmap.isNull()) return pixmap;

    QImage qimg = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(qimg.height(), qimg.width(), CV_8UC3,
                (uchar*)qimg.bits(), qimg.bytesPerLine());

    if (level == 0) {
        return pixmap; // 원본
    }

    cv::Mat result;
    if (level > 0) {
        // CLAHE로 대비 강화
        cv::Mat labImg;
        cv::cvtColor(mat, labImg, cv::COLOR_RGB2Lab);
        std::vector<cv::Mat> labPlanes(3);
        cv::split(labImg, labPlanes);

        int clipLimit = std::min(2 + level / 20, 10); // level=100 → 약 clipLimit=7
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, cv::Size(8, 8));
        clahe->apply(labPlanes[0], labPlanes[0]);

        cv::merge(labPlanes, labImg);
        cv::cvtColor(labImg, result, cv::COLOR_Lab2RGB);
    } else {
        // 음수 → 대비 약화 (가벼운 블러)
        int ksize = std::max(1, -level / 20 * 2 + 1);
        cv::GaussianBlur(mat, result, cv::Size(ksize, ksize), 0);
    }

    QImage resultImg((uchar*)result.data,
                     result.cols, result.rows,
                     result.step,
                     QImage::Format_RGB888);
    return QPixmap::fromImage(resultImg.copy());
}
