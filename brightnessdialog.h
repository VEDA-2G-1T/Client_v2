#ifndef BRIGHTNESSDIALOG_H
#define BRIGHTNESSDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include "camerainfo.h"

class BrightnessDialog : public QDialog {
    Q_OBJECT

public:
    explicit BrightnessDialog(const QVector<CameraInfo> &cameras, QWidget *parent = nullptr);

signals:
    void brightnessConfirmed(const CameraInfo &camera, int value);

private:
    QComboBox *cameraSelector;
    QSlider *slider;
    QLabel *valueLabel;
    QVector<CameraInfo> cameraList;
};

#endif // BRIGHTNESSDIALOG_H
