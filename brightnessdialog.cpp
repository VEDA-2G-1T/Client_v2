#include "brightnessdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFontDatabase>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>

BrightnessDialog::BrightnessDialog(const QVector<CameraInfo> &cameras, QWidget *parent)
    : QDialog(parent), cameraList(cameras)
{
    setWindowTitle("카메라 밝기 조절");
    setFixedSize(400, 240);
    setStyleSheet("background-color: #2b2b2b; color: white;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 🔹 카메라 선택 콤보박스
    cameraSelector = new QComboBox();
    for (const CameraInfo &cam : cameraList) {
        cameraSelector->addItem(QString("%1 (%2)").arg(cam.name, cam.ip));
    }
    cameraSelector->setStyleSheet(R"(
        QComboBox {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            border-radius: 4px;
            padding: 4px;
        }
    )");
    mainLayout->addWidget(cameraSelector);

    // 🔹 슬라이더 라벨 + 현재 값
    QHBoxLayout *topLayout = new QHBoxLayout();
    QLabel *label = new QLabel("밝기 (-255 ~ 255):");
    valueLabel = new QLabel("0");
    label->setStyleSheet("color: white;");
    valueLabel->setStyleSheet("color: orange; font-weight: bold;");
    topLayout->addWidget(label);
    topLayout->addStretch();
    topLayout->addWidget(valueLabel);

    mainLayout->addLayout(topLayout);

    // 🔹 슬라이더
    slider = new QSlider(Qt::Horizontal);
    slider->setRange(-255, 255);
    slider->setValue(0);
    slider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            border: 1px solid #444;
            height: 6px;
            background: #2e2e2e;
            margin: 0px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #f37321;
            border: 1px solid #444;
            width: 12px;
            margin: -4px 0;
            border-radius: 6px;
        }
    )");

    connect(slider, &QSlider::valueChanged, this, [=](int val) {
        valueLabel->setText(QString::number(val));
    });

    mainLayout->addWidget(slider);

    // 🔹 슬라이더 하단 범위 표시
    QHBoxLayout *rangeLayout = new QHBoxLayout();
    QLabel *minLabel = new QLabel("-255");
    QLabel *midLabel = new QLabel("0");
    QLabel *maxLabel = new QLabel("+255");

    QFont labelFont;
    labelFont.setPointSize(9);
    minLabel->setFont(labelFont);
    midLabel->setFont(labelFont);
    maxLabel->setFont(labelFont);

    minLabel->setStyleSheet("color: gray;");
    midLabel->setStyleSheet("color: gray;");
    maxLabel->setStyleSheet("color: gray;");

    rangeLayout->addWidget(minLabel, 1);
    rangeLayout->addWidget(midLabel, 1, Qt::AlignHCenter);
    rangeLayout->addWidget(maxLabel, 1, Qt::AlignRight);

    mainLayout->addLayout(rangeLayout);

    // 🔹 적용 버튼
    QPushButton *okBtn = new QPushButton("적용");
    okBtn->setFixedHeight(30);
    okBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #f37321;
            border: 1px solid #f37321;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #f37321;
            color: white;
        }
    )");

    connect(okBtn, &QPushButton::clicked, this, [=]() {
        int idx = cameraSelector->currentIndex();
        if (idx >= 0 && idx < cameraList.size()) {
            emit brightnessConfirmed(cameraList[idx], slider->value());
        }
        accept();
    });

    mainLayout->addWidget(okBtn);
}
