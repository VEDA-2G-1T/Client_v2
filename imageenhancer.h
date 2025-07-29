#ifndef IMAGEENHANCER_H
#define IMAGEENHANCER_H

#include <QPixmap>

class ImageEnhancer {
public:
    static QPixmap enhanceSharpness(const QPixmap &pixmap, int level);
    static QPixmap enhanceCLAHE(const QPixmap &pixmap, int clipLimit = 2);
};

#endif // IMAGEENHANCER_H
