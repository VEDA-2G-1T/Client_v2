#ifndef BRIGHTNESSDIALOG_H
#define BRIGHTNESSDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QMouseEvent>
#include "camerainfo.h"

class BrightnessDialog : public QDialog {
    Q_OBJECT

public:
    explicit BrightnessDialog(const QVector<CameraInfo> &cameras, QWidget *parent = nullptr);

signals:
    void brightnessConfirmed(const CameraInfo &camera, int value);

protected:
    // ✅ 드래그 이동을 위한 마우스 이벤트 처리
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QComboBox *cameraSelector;
    QSlider *slider;
    QLabel *valueLabel;
    QVector<CameraInfo> cameraList;

    // ✅ 드래그 상태 변수
    bool dragging = false;
    QPoint dragPosition;
};

#endif // BRIGHTNESSDIALOG_H
