#include "cameraitemwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QFontDatabase>

CameraItemWidget::CameraItemWidget(const CameraInfo &info, QWidget *parent)
    : QWidget(parent), camera(info)
{
    // ✅ 폰트 등록
    static int gidL = QFontDatabase::addApplicationFont(":/resources/fonts/06HanwhaGothicL.ttf");
    static int gidR = QFontDatabase::addApplicationFont(":/resources/fonts/05HanwhaGothicR.ttf");
    QString gfontL = QFontDatabase::applicationFontFamilies(gidL).value(0);
    QString gfontR = QFontDatabase::applicationFontFamilies(gidR).value(0);

    QFont labelFont(gfontL, 10);
    QFont comboFont(gfontR, 8);

    // ✅ 라벨: 이름 (IP)
    label = new QLabel(QString("%1 (%2)").arg(info.name, info.ip));
    label->setFont(labelFont);
    label->setStyleSheet("color: white;");

    // ✅ 삭제 버튼
    removeButton = new QPushButton();
    removeButton->setToolTip("카메라 삭제");
    removeButton->setIcon(QIcon(":/resources/icons/delete.png"));
    removeButton->setIconSize(QSize(16, 16));
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

    connect(removeButton, &QPushButton::clicked, this, &CameraItemWidget::onRemoveClicked);

    // ✅ 모드 드롭다운
    comboBox = new QComboBox();
    comboBox->addItems({"Raw", "Blur", "Detect", "Trespass", "Fall"});
    comboBox->setFont(comboFont);
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
        }
    )");

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraItemWidget::onModeChanged);

    // ✅ 상태 표시 라벨
    statusLabel = new QLabel("");
    statusLabel->setStyleSheet("color: lightgray; font-size: 10px;");
    statusLabel->setFixedWidth(180);

    // ✅ 상단: 라벨 + 삭제 버튼
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(label);
    topLayout->addStretch();
    topLayout->addWidget(removeButton);

    // ✅ 하단: 드롭다운 + 상태 라벨
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(comboBox);
    bottomLayout->addWidget(statusLabel);
    bottomLayout->addStretch();

    // ✅ 전체 수직 배치
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
    mainLayout->setContentsMargins(5, 2, 5, 2);
    mainLayout->setSpacing(4);
}

void CameraItemWidget::onModeChanged(int index)
{
    QString mode = comboBox->itemText(index).toLower();
    emit modeChanged(mode, camera);
}

void CameraItemWidget::onRemoveClicked()
{
    emit removeRequested(camera);
}

CameraInfo CameraItemWidget::getCameraInfo() const
{
    return camera;
}

/*
// ✅ 상태 표시 라벨 텍스트/색상 변경 함수
void CameraItemWidget::updateHealthStatus(const QString &text, const QString &color)
{
    statusLabel->setText(text);
    statusLabel->setStyleSheet(QString("color: %1; font-size: 10px;").arg(color));
}
*/

void CameraItemWidget::updateHealthStatus(const QString &text, const QString &color)
{
    statusLabel->setText(text);

    // ✅ 폰트 설정
    static int gid = QFontDatabase::addApplicationFont(":/resources/fonts/05HanwhaGothicR.ttf");
    static QString fontFamily = QFontDatabase::applicationFontFamilies(gid).value(0);
    QFont font(fontFamily, 10);
    statusLabel->setFont(font);

    // ✅ 색상만 스타일로
    statusLabel->setStyleSheet(QString("color: %1;").arg(color));
}

