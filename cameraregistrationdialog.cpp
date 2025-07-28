#include "cameraregistrationdialog.h"
#include "camerainfo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>
#include <QFontDatabase>

CameraRegistrationDialog::CameraRegistrationDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("카메라 등록");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);  // ✅ 타이틀바 제거
    setFixedSize(420, 300);
    setModal(true);

    setStyleSheet(R"(
        QDialog {
            background-color: #1e1e1e;
            color: white;
        }
        QLabel {
            color: white;
        }
        QLineEdit {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 5px;
            border-radius: 4px;
            font-size: 13px;
        }
        QPushButton {
            font-size: 12px;
            background-color: #2b2b2b;
            color: white;
            border: 1px solid #444;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
    )");
}

void CameraRegistrationDialog::setupUI()
{
    int gidR = QFontDatabase::addApplicationFont(":/resources/fonts/05HanwhaGothicR.ttf");
    int gidL = QFontDatabase::addApplicationFont(":/resources/fonts/06HanwhaGothicL.ttf");

    QString gfontR = QFontDatabase::applicationFontFamilies(gidR).at(0);
    QString gfontL = QFontDatabase::applicationFontFamilies(gidL).at(0);

    QFont labelFont(gfontR, 10);
    QFont inputFont(gfontL, 10);
    QFont buttonFont(gfontL, 8);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *nameLabel = new QLabel("카메라 이름:");
    nameLabel->setFont(labelFont);
    nameEdit = new QLineEdit();
    nameEdit->setFont(inputFont);
    nameEdit->setPlaceholderText("카메라가 설치된 장소를 입력하세요.");

    QLabel *ipLabel = new QLabel("카메라 IP:");
    ipLabel->setFont(labelFont);
    ipEdit = new QLineEdit();
    ipEdit->setFont(inputFont);
    ipEdit->setPlaceholderText("예: 192.168.0.87");

    QRegularExpression ipRegex("^(?:(?:25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)$");
    QValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);
    ipEdit->setValidator(ipValidator);

    QLabel *portLabel = new QLabel("포트번호:");
    portLabel->setFont(labelFont);
    portEdit = new QLineEdit();
    portEdit->setFont(inputFont);
    portEdit->setPlaceholderText("미입력시 카메라 기본설정인 8555가 고정됩니다.");

    okButton = new QPushButton("등록");
    okButton->setFont(buttonFont);
    cancelButton = new QPushButton("취소");
    cancelButton->setFont(buttonFont);
    okButton->setFixedSize(60, 30);
    cancelButton->setFixedSize(60, 30);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okButton);
    btnLayout->addWidget(cancelButton);

    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(nameEdit);
    mainLayout->addWidget(ipLabel);
    mainLayout->addWidget(ipEdit);
    mainLayout->addWidget(portLabel);
    mainLayout->addWidget(portEdit);
    mainLayout->addLayout(btnLayout);

    connect(okButton, &QPushButton::clicked, this, &CameraRegistrationDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &CameraRegistrationDialog::onCancelClicked);
}

void CameraRegistrationDialog::onOkClicked()
{
    if (nameEdit->text().trimmed().isEmpty() || ipEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "모든 항목을 입력해주세요.");
        return;
    }

    if (portEdit->text().trimmed().isEmpty()) {
        portEdit->setText("8555");
    }

    accept();
}

void CameraRegistrationDialog::onCancelClicked()
{
    reject();
}

// ✅ 드래그 기능 추가
void CameraRegistrationDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void CameraRegistrationDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void CameraRegistrationDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = false;
        event->accept();
    }
}

// Getter
QString CameraRegistrationDialog::getCameraName() const { return nameEdit->text().trimmed(); }
QString CameraRegistrationDialog::getCameraIP() const { return ipEdit->text().trimmed(); }
QString CameraRegistrationDialog::getCameraPort() const { return portEdit->text().trimmed(); }
