#include "cameraitemwidget.h"
#include <QHBoxLayout>

CameraItemWidget::CameraItemWidget(const CameraInfo &info, QWidget *parent)
    : QWidget(parent), camera(info)
{
    label = new QLabel(QString("%1 (%2:%3)").arg(info.name, info.ip, info.port));
    label->setStyleSheet("color: white;");

    comboBox = new QComboBox();
    comboBox->addItems({"Raw", "Blur", "Detect", "Trespass", "Fall"});
    comboBox->setFixedWidth(100);
    comboBox->setStyleSheet(R"(
        QComboBox {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            border-radius: 4px;
            padding: 2px;
        }
        QComboBox QAbstractItemView {
        background-color: #2b2b2b;
        color: white;
        selection-background-color: #505050;
        selection-color: white;
        }
    )");

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraItemWidget::onModeChanged);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(comboBox);
    layout->setContentsMargins(5, 2, 5, 2);
}

void CameraItemWidget::onModeChanged(int index)
{
    QString mode = comboBox->itemText(index).toLower();  // e.g., "raw"
    emit modeChanged(mode, camera);
}

CameraInfo CameraItemWidget::getCameraInfo() const
{
    return camera;
}
