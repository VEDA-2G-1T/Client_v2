#include "cameraitemwidget.h"
#include <QHBoxLayout>
#include <QIcon>

/*
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

    removeButton = new QPushButton();
    removeButton->setIcon(QIcon(":/resources/icons/delete.png")); // 아이콘 경로는 예시입니다
    removeButton->setToolTip("카메라 삭제");
    removeButton->setFixedSize(24, 24);
    removeButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
        }
        QPushButton:hover {
            background-color: #800000;
        }
    )");

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraItemWidget::onModeChanged);

    connect(removeButton, &QPushButton::clicked,
            this, &CameraItemWidget::onRemoveClicked);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(comboBox);
    layout->addWidget(removeButton); // ✅ 추가
    layout->setContentsMargins(5, 2, 5, 2);
}
*/

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

    // ✅ 삭제 버튼
    removeButton = new QPushButton();
    removeButton->setToolTip("카메라 삭제");
    removeButton->setIcon(QIcon(":/resources/icons/delete.png"));  // 리소스 등록 필요
    removeButton->setIconSize(QSize(16, 16));
    removeButton->setFixedSize(24, 24);
    removeButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
        }
    )");

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraItemWidget::onModeChanged);
    connect(removeButton, &QPushButton::clicked,
            this, &CameraItemWidget::onRemoveClicked);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(comboBox);
    layout->addWidget(removeButton);
    layout->setContentsMargins(5, 2, 5, 2);
}

void CameraItemWidget::onModeChanged(int index)
{
    QString mode = comboBox->itemText(index).toLower();
    emit modeChanged(mode, camera);
}

void CameraItemWidget::onRemoveClicked()
{
    emit removeRequested(camera);  // ✅ 카메라 제거 요청
}

CameraInfo CameraItemWidget::getCameraInfo() const
{
    return camera;
}
