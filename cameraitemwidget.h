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

    // ✅ 헬시 상태 업데이트용 함수
    void updateHealthStatus(const QString &text, const QString &color = "lightgray");

signals:
    void modeChanged(const QString &mode, const CameraInfo &camera);
    void removeRequested(const CameraInfo &camera);

private slots:
    void onModeChanged(int index);
    void onRemoveClicked();

private:
    CameraInfo camera;
    QLabel *label;
    QComboBox *comboBox;
    QPushButton *removeButton;
    QLabel *statusLabel;  // ✅ 드롭다운 옆에 헬시 상태 표시용
};

#endif // CAMERAITEMWIDGET_H
