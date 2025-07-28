#include "brightnessdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFontDatabase>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QMouseEvent>

BrightnessDialog::BrightnessDialog(const QVector<CameraInfo> &cameras, QWidget *parent)
    : QDialog(parent), cameraList(cameras)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);  // ✅ 심리스 다이얼로그
    setFixedSize(300, 180);
    setStyleSheet("background-color: #2b2b2b; color: white;");

    int rid = QFontDatabase::addApplicationFont(":/resources/fonts/05HanwhaGothicR.ttf");
    int lid = QFontDatabase::addApplicationFont(":/resources/fonts/06HanwhaGothicL.ttf");
    QString rFont = QFontDatabase::applicationFontFamilies(rid).at(0);
    QString lFont = QFontDatabase::applicationFontFamilies(lid).at(0);

    QFont labelFont(rFont, 10);
    QFont valueFont(rFont, 10, QFont::Bold);
    QFont comboFont(lFont, 10);
    QFont rangeFont(lFont, 8);
    QFont buttonFont(lFont, 9);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(2);

    // 🔹 상단 닫기 버튼
    QWidget *topBar = new QWidget();
    topBar->setFixedHeight(20);
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(20, 20);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: white;
            border: none;
        }
        QPushButton:hover {
            color: #f37321;
        }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    topBarLayout->addStretch();
    topBarLayout->addWidget(closeBtn);
    mainLayout->addWidget(topBar);

    // 🔹 카메라 선택 섹션
    QWidget *cameraSection = new QWidget();
    cameraSection->setFixedHeight(28);
    QHBoxLayout *cameraLayout = new QHBoxLayout(cameraSection);
    cameraLayout->setContentsMargins(2, 2, 2, 2);
    cameraLayout->setSpacing(1);

    QLabel *cameraLabel = new QLabel("카메라 선택");
    cameraLabel->setStyleSheet("color: white;");
    cameraLabel->setFont(labelFont);

    cameraSelector = new QComboBox();
    for (const CameraInfo &cam : cameraList) {
        cameraSelector->addItem(QString("%1 (%2)").arg(cam.name, cam.ip));
    }
    cameraSelector->setFont(comboFont);
    cameraSelector->setStyleSheet(R"(
        QComboBox {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            border-radius: 4px;
            padding: 4px;
        }
    )");

    cameraLayout->addWidget(cameraLabel);
    cameraLayout->addWidget(cameraSelector);
    mainLayout->addWidget(cameraSection);

    // 🔹 밝기 레이블 + 현재값
    QWidget *labelSection = new QWidget();
    labelSection->setFixedHeight(24);
    QHBoxLayout *topLayout = new QHBoxLayout(labelSection);
    topLayout->setContentsMargins(2, 2, 2, 2);
    topLayout->setSpacing(1);

    QLabel *label = new QLabel("밝기 (-255 ~ 255)");
    valueLabel = new QLabel("0");
    label->setStyleSheet("color: white;");
    valueLabel->setStyleSheet("color: orange; font-weight: bold;");
    label->setFont(labelFont);
    valueLabel->setFont(valueFont);

    topLayout->addWidget(label);
    topLayout->addStretch();
    topLayout->addWidget(valueLabel);
    mainLayout->addWidget(labelSection);

    // 🔹 슬라이더
    QWidget *sliderSection = new QWidget();
    sliderSection->setFixedHeight(28);
    QVBoxLayout *sliderLayout = new QVBoxLayout(sliderSection);
    sliderLayout->setContentsMargins(4, 4, 4, 4);

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

    sliderLayout->addWidget(slider);
    mainLayout->addWidget(sliderSection);

    // 🔹 슬라이더 범위 표시
    QWidget *rangeSection = new QWidget();
    rangeSection->setFixedHeight(18);
    QHBoxLayout *rangeLayout = new QHBoxLayout(rangeSection);
    rangeLayout->setContentsMargins(0, 0, 0, 0);
    rangeLayout->setSpacing(0);

    QLabel *minLabel = new QLabel("-255");
    QLabel *midLabel = new QLabel("0");
    QLabel *maxLabel = new QLabel("+255");

    minLabel->setFont(rangeFont);
    midLabel->setFont(rangeFont);
    maxLabel->setFont(rangeFont);

    minLabel->setStyleSheet("color: gray;");
    midLabel->setStyleSheet("color: gray;");
    maxLabel->setStyleSheet("color: gray;");

    rangeLayout->addWidget(minLabel, 1);
    rangeLayout->addWidget(midLabel, 1, Qt::AlignHCenter);
    rangeLayout->addWidget(maxLabel, 1, Qt::AlignRight);
    mainLayout->addWidget(rangeSection);

    // 🔹 적용 버튼
    QWidget *buttonSection = new QWidget();
    buttonSection->setFixedHeight(36);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonSection);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();

    QPushButton *okBtn = new QPushButton("적용");
    okBtn->setFixedHeight(28);
    okBtn->setFont(buttonFont);
    okBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #f37321;
            border: 1px solid #f37321;
            border-radius: 4px;
            padding: 4px 10px;
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

    buttonLayout->addWidget(okBtn);
    buttonLayout->addStretch();
    mainLayout->addWidget(buttonSection);
}

// ✅ 드래그 이동 기능 추가
void BrightnessDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void BrightnessDialog::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}
void BrightnessDialog::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging = false;
        event->accept();
    }
}
