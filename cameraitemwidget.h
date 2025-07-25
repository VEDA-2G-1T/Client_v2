#ifndef CAMERAITEMWIDGET_H
#define CAMERAITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "camerainfo.h"

class CameraItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraItemWidget(const CameraInfo &info, QWidget *parent = nullptr);
    CameraInfo getCameraInfo() const;

signals:
    void modeChanged(const QString &mode, const CameraInfo &camera);
    void removeRequested(const CameraInfo &camera);  // ✅ 추가

private slots:
    void onModeChanged(int index);
    void onRemoveClicked(); // ✅ 추가

private:
    CameraInfo camera;
    QLabel *label;
    QComboBox *comboBox;
    QPushButton *removeButton; // ✅ 추가
};

#endif // CAMERAITEMWIDGET_H
