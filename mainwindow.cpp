#include "mainwindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QDateTime>
#include <QTimer>
#include <QTableWidget>
#include <QHeaderView>
#include <QGridLayout>
#include <QSpacerItem>
#include <QStyle>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Smart SafetyNet");
    resize(1560, 720);

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupTopBar();
    setupCameraList();
    setupVideoGrid();
    setupEventLog();

    mainLayout->addWidget(topBar);

    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);  // 여백 제거

    bodyLayout->addWidget(cameraListWrapper, 1);
    bodyLayout->addWidget(videoGridPanel, 3);
    bodyLayout->addWidget(eventLogPanel, 2);

    mainLayout->addLayout(bodyLayout);
    setCentralWidget(central);
}

void MainWindow::setupTopBar() {
    topBar = new QWidget();
    topBar->setFixedHeight(50);
    QHBoxLayout *layout = new QHBoxLayout(topBar);

    QLabel *logoLabel = new QLabel("🛡 Smart SafetyNet");
    logoLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: orange;");

    timeLabel = new QLabel();
    timeLabel->setStyleSheet("font-size: 14px; color: white;");

    QPushButton *closeButton = new QPushButton("종료");

    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

    layout->addWidget(logoLabel);
    layout->addStretch();
    layout->addWidget(timeLabel);
    layout->addSpacing(10);
    layout->addWidget(closeButton);

    topBar->setStyleSheet("background-color: #1e1e1e; color: white;");

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        timeLabel->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    });
    timer->start(1000);
}

void MainWindow::setupCameraList() {
    cameraListWrapper = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(cameraListWrapper);
    cameraListWrapper->setStyleSheet("background-color: #2b2b2b;");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 🔘 상단 아이콘 버튼 3개
    QHBoxLayout *iconLayout = new QHBoxLayout();
    iconLayout->setSpacing(0);
    iconLayout->setContentsMargins(0, 0, 0, 0);  // 여백 제거

    auto createIconButton = [](const QString &iconPath, const QString &tooltip) -> QPushButton* {
        QPushButton *btn = new QPushButton();
        btn->setIcon(QIcon(iconPath));
        btn->setIconSize(QSize(32, 32));
        btn->setToolTip(tooltip);

        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setMinimumHeight(48);

        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #1e1e1e;
                border: none;
            }
            QPushButton:hover {
                background-color: #f37321;
            }
        )");
        return btn;
    };

    // ✅ 버튼 선언
    QPushButton *cameraButton = createIconButton(":/resources/icons/camera_registration.png", "카메라 등록");
    QPushButton *settingsButton = createIconButton(":/resources/icons/settings.png", "설정");
    QPushButton *healthButton = createIconButton(":/resources/icons/health_check.png", "헬시 체크");

    // ✅ 연결: 카메라 등록 버튼 클릭 시 다이얼로그 띄우기
    connect(cameraButton, &QPushButton::clicked, this, [=]() {
        CameraRegistrationDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString name = dialog.getCameraName();
            QString ip = dialog.getCameraIP();
            QString port = dialog.getCameraPort();
            QString display = QString("%1 (%2:%3)").arg(name, ip, port);
            cameraListPanel->addItem(display);
        }
    });

    // 버튼 레이아웃에 추가
    iconLayout->addWidget(cameraButton);
    iconLayout->addWidget(settingsButton);
    iconLayout->addWidget(healthButton);

    // 📋 카메라 리스트
    cameraListPanel = new QListWidget();
    cameraListPanel->setStyleSheet(R"(
        QListWidget {
            background-color: #1e1e1e;
            color: white;
            border: none;
        }
        QListWidget::item:selected {
            background-color: #2a82da;
        }
    )");
    cameraListPanel->setFocusPolicy(Qt::NoFocus);
    cameraListPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 전체 구성 정리
    layout->addLayout(iconLayout);
    layout->addWidget(cameraListPanel);
    cameraListWrapper->setFixedWidth(200);
}

void MainWindow::setupVideoGrid() {
    videoGridPanel = new QWidget();
    videoGridPanel->setStyleSheet("background-color: #2b2b2b;");
    videoGridPanel->setMinimumSize(960, 720);
    videoGridPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGridLayout *grid = new QGridLayout(videoGridPanel);
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);

    // 📌 각 위치에 대응되는 CAMERA 라벨
    QMap<QPair<int, int>, QString> cameraLabels = {
        {{0, 0}, "CAMERA #1"},
        {{0, 1}, "CAMERA #2"},
        {{0, 2}, "CAMERA #4"},
        {{1, 0}, "CAMERA #3"},
        {{2, 0}, "CAMERA #5"}
    };

    const QSize tileMinSize(320, 240);
    for (int row : {0, 1, 2}) {
        for (int col : {0, 1, 2}) {
            if ((row == 1 && col == 1) || (row == 1 && col == 2) ||
                (row == 2 && col == 1) || (row == 2 && col == 2))
                continue;

            QString labelText = cameraLabels.value({row, col}, QString("(%1,%2)").arg(row).arg(col));
            QLabel *tile = new QLabel(labelText);
            tile->setStyleSheet(R"(
                background-color: black;
                color: white;
                border: 1px solid gray;
                margin: 0px;
                padding: 0px;
            )");
            tile->setAlignment(Qt::AlignCenter);
            tile->setMinimumSize(tileMinSize);
            tile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            tile->setScaledContents(true);
            grid->addWidget(tile, row, col);
        }
    }

    // ✅ 4:3 비율로 고정해서 보여줄 ONVIF 영상 타일
    QWidget *videoContainer = new QWidget();
    videoContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVideoWidget *videoWidget = new QVideoWidget(videoContainer);
    videoWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    videoWidget->setGeometry(0, 0, 640, 480);
    videoWidget->setStyleSheet("background-color: black; border: 1px solid gray;");
    videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);  // 왜곡 허용

    // EventFilter를 통해 resize 시 강제 4:3 비율 유지
    videoContainer->installEventFilter(this);

    QMediaPlayer *player = new QMediaPlayer(this);
    player->setVideoOutput(videoWidget);
    player->setSource(QUrl("rtsp://192.168.0.35:554/0/onvif/profile2/media.smp"));
    player->play();

    grid->addWidget(videoContainer, 1, 1, 2, 2);

    // ✅ 비율 유지하도록 stretch 적용
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 1);

    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 1);
    grid->setRowStretch(2, 1);
}

void MainWindow::setupEventLog() {
    eventLogPanel = new QTableWidget(0, 3);
    QStringList headers = {"Time", "Camera", "Event"};
    eventLogPanel->setHorizontalHeaderLabels(headers);
    eventLogPanel->horizontalHeader()->setStretchLastSection(true);
    eventLogPanel->setEditTriggers(QAbstractItemView::NoEditTriggers);
    eventLogPanel->verticalHeader()->setVisible(false);
    eventLogPanel->setFixedWidth(400);
    eventLogPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    eventLogPanel->setStyleSheet("background-color: #1e1e1e; color: white;");
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::Resize) {
        QWidget *container = qobject_cast<QWidget *>(watched);
        if (container) {
            QSize size = container->size();
            int w = size.width();
            int h = size.height();

            // 4:3 비율 유지
            float targetAspect = 4.0f / 3.0f;
            int newW = w;
            int newH = static_cast<int>(w / targetAspect);

            if (newH > h) {
                newH = h;
                newW = static_cast<int>(h * targetAspect);
            }

            QVideoWidget *video = container->findChild<QVideoWidget *>();
            if (video)
                video->setGeometry((w - newW) / 2, (h - newH) / 2, newW, newH);
        }
    }
    return false;
}
