#ifndef IMAGEENHANCER_H
#define IMAGEENHANCER_H

#include <QPixmap>

class ImageEnhancer {
public:
    static QPixmap enhanceSharpness(const QPixmap &pixmap);
    static QPixmap enhanceCLAHE(const QPixmap &pixmap);
};

#endif // IMAGEENHANCER_H
