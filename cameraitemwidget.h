#ifndef CAMERAITEMWIDGET_H
#define CAMERAITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include "camerainfo.h"

class CameraItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraItemWidget(const CameraInfo &info, QWidget *parent = nullptr);
    CameraInfo getCameraInfo() const;

signals:
    void modeChanged(const QString &mode, const CameraInfo &camera);

private slots:
    void onModeChanged(int index);

private:
    CameraInfo camera;
    QLabel *label;
    QComboBox *comboBox;
};

#endif // CAMERAITEMWIDGET_H
