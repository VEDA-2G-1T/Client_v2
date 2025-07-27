#ifndef IMAGEENHANCER_H
#define IMAGEENHANCER_H

#include <QImage>
#include <QByteArray>

class ImageEnhancer
{
public:
    static QImage enhance(const QByteArray &imageData);
};

#endif // IMAGEENHANCER_H
