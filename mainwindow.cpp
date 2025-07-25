#include "mainwindow.h"
#include "camerainfo.h"
#include "cameraitemwidget.h"
#include "cameraregistrationdialog.h"

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
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtWebSockets/QWebSocket>

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

QPair<int, int> MainWindow::findEmptyVideoSlot() {
    QList<QPair<int, int>> reservedSlots = {
        {0, 0}, {0, 1}, {0, 2},
        {1, 0}, {2, 0}
    };

    QGridLayout *grid = qobject_cast<QGridLayout *>(videoGridPanel->layout());
    if (!grid) return {-1, -1};

    for (const QPair<int, int> &pos : reservedSlots) {
        QLayoutItem *item = grid->itemAtPosition(pos.first, pos.second);
        if (!item) return pos;

        QWidget *widget = item->widget();
        if (widget && widget->objectName() == "placeholder")
            return pos;
    }

    return {-1, -1};
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
    if (!cameraListWrapper) {
        cameraListWrapper = new QWidget();
        cameraListWrapper->setFixedWidth(300);
        cameraListWrapper->setStyleSheet("background-color: #2b2b2b;");

        wrapperLayout = new QVBoxLayout(cameraListWrapper);
        wrapperLayout->setContentsMargins(0, 0, 0, 0);
        wrapperLayout->setSpacing(0);

        // 상단 버튼 영역
        QHBoxLayout *iconLayout = new QHBoxLayout();
        iconLayout->setSpacing(0);
        iconLayout->setContentsMargins(0, 0, 0, 0);

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

        QPushButton *cameraButton = createIconButton(":/resources/icons/camera_registration.png", "카메라 등록");
        QPushButton *settingsButton = createIconButton(":/resources/icons/settings.png", "설정");
        QPushButton *healthButton = createIconButton(":/resources/icons/health_check.png", "헬시 체크");

        iconLayout->addWidget(cameraButton);
        iconLayout->addWidget(settingsButton);
        iconLayout->addWidget(healthButton);
        wrapperLayout->addLayout(iconLayout);

        // 리스트 영역
        listContainer = new QWidget();
        listLayout = new QVBoxLayout(listContainer);
        listLayout->setContentsMargins(0, 0, 0, 0);
        listLayout->setSpacing(2);

        scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(listContainer);
        scrollArea->setStyleSheet("background-color: #1e1e1e;");
        scrollArea->setFrameStyle(QFrame::NoFrame);
        wrapperLayout->addWidget(scrollArea);

        connect(cameraButton, &QPushButton::clicked, this, [=]() {
            CameraRegistrationDialog dialog(this);
            if (dialog.exec() == QDialog::Accepted) {
                QString name = dialog.getCameraName();
                QString ip = dialog.getCameraIP();
                QString port = dialog.getCameraPort();

                CameraInfo newCam{name, ip, port};
                cameraList.append(newCam);
                qDebug() << "[등록] 새 카메라 추가:" << name << ip << port;

                QPair<int, int> pos = findEmptyVideoSlot();
                if (pos.first == -1) {
                    QMessageBox::warning(this, "배치 불가", "모든 영상 슬롯이 사용 중입니다.");
                    return;
                }

                QGridLayout *grid = qobject_cast<QGridLayout *>(videoGridPanel->layout());
                if (!grid) return;

                QLayoutItem *existingItem = grid->itemAtPosition(pos.first, pos.second);
                if (existingItem && existingItem->widget()) {
                    QWidget *oldWidget = existingItem->widget();
                    grid->removeWidget(oldWidget);
                    delete oldWidget;
                }

                // ✅ 영상 출력용 tile wrapper
                QWidget *tileWrapper = new QWidget();
                tileWrapper->setFixedSize(320, 240);
                tileWrapper->setStyleSheet("background-color: black;");

                tileWrapper->setProperty("camera_ip", ip);  // 이 줄이 꼭 있어야 합니다

                QGraphicsScene *scene = new QGraphicsScene(tileWrapper);
                QGraphicsVideoItem *videoItem = new QGraphicsVideoItem();
                videoItem->setSize(QSizeF(320, 240));
                scene->addItem(videoItem);

                // ✅ 이름 라벨 + 검정 배경 박스
                QGraphicsRectItem *labelBg = scene->addRect(0, 0, 0, 0, Qt::NoPen, QBrush(QColor(0, 0, 0, 180)));
                QGraphicsTextItem *labelItem = scene->addText(name);
                labelItem->setDefaultTextColor(Qt::white);
                labelItem->setFont(QFont("Arial", 10, QFont::Bold));
                labelItem->setZValue(2);  // 텍스트 위
                labelBg->setZValue(1);    // 박스 뒤

                QRectF textRect = labelItem->boundingRect();
                labelBg->setRect(0, 0, textRect.width() + 10, textRect.height() + 4);
                labelBg->setPos(320 - textRect.width() - 14, 5);
                labelItem->setPos(320 - textRect.width() - 9, 7);

                QGraphicsView *view = new QGraphicsView(scene, tileWrapper);
                view->setFixedSize(320, 240);
                view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                view->setStyleSheet("border: none; background: black;");
                view->setFrameStyle(QFrame::NoFrame);

                QMediaPlayer *player = new QMediaPlayer(this);
                player->setVideoOutput(videoItem);
                player->setSource(QUrl(QString("rtsps://%1:%2/processed").arg(ip, port)));
                player->play();

                players.append(player);
                grid->addWidget(tileWrapper, pos.first, pos.second);

                // ✅ 리스트 갱신 및 WebSocket 연결
                refreshCameraListItems();
                setupWebSocketConnections();
            }
        });


    }

    refreshCameraListItems(); // 초기화 시 최초 1회 호출
}

void MainWindow::refreshCameraListItems() {
    if (!listLayout) return;

    // 기존 리스트 아이템 삭제
    QLayoutItem *child;
    while ((child = listLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    qDebug() << "[갱신] 카메라 리스트 항목 갱신 시작, 항목 수:" << cameraList.size();

    // 새 항목 추가
    for (const CameraInfo &cam : cameraList) {
        qDebug() << "[갱신] 카메라 추가:" << cam.name << cam.ip << cam.port;

        CameraItemWidget *item = new CameraItemWidget(cam);

        // 🔄 모드 변경 시 WebSocket 메시지 전송
        connect(item, QOverload<const QString &, const CameraInfo &>::of(&CameraItemWidget::modeChanged),
                this, &MainWindow::sendModeChangeRequest);

        // 🗑 삭제 요청 처리
        connect(item, &CameraItemWidget::removeRequested, this, [=](const CameraInfo &target) {
            qDebug() << "[삭제 요청] 카메라 제거:" << target.name << target.ip << target.port;

            // 1. cameraList에서 제거
            cameraList.removeOne(target);

            // 2. videoGridPanel에서 해당 타일 제거
            QGridLayout *grid = qobject_cast<QGridLayout *>(videoGridPanel->layout());
            if (grid) {
                for (int row = 0; row < grid->rowCount(); ++row) {
                    for (int col = 0; col < grid->columnCount(); ++col) {
                        QLayoutItem *item = grid->itemAtPosition(row, col);
                        if (item) {
                            QWidget *widget = item->widget();
                            if (widget && widget->property("camera_ip").toString() == target.ip) {
                                grid->removeWidget(widget);
                                widget->deleteLater();
                                qDebug() << "[타일 제거] 위치:" << row << col;
                                // ✅ 자리 복구용 placeholder 생성
                                QLabel *placeholder = new QLabel(QString("CAMERA #%1").arg(row * 3 + col + 1));
                                placeholder->setObjectName("placeholder");
                                placeholder->setAlignment(Qt::AlignCenter);
                                placeholder->setMinimumSize(320, 240);
                                placeholder->setStyleSheet(R"(
                                    background-color: rgba(0, 0, 0, 180);
                                    color: white;
                                    font-weight: bold;
                                    border-radius: 5px;
                                    font-size: 12px;
                                )");
                                grid->addWidget(placeholder, row, col);
                            }
                        }
                    }
                }
            }

            // 3. WebSocket 정리
            if (socketMap.contains(target.ip)) {
                QWebSocket *sock = socketMap[target.ip];
                sock->close();
                sock->deleteLater();
                socketMap.remove(target.ip);
                qDebug() << "[WebSocket 제거]" << target.ip;
            }

            // 리스트 다시 갱신
            refreshCameraListItems();
        });

        listLayout->addWidget(item);
    }

    listLayout->addStretch();  // 아래 빈 공간 채움
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
            tile->setObjectName("placeholder");  // ✅ 자리 표시자임을 명시
            tile->setAutoFillBackground(true);
            tile->setStyleSheet(R"(
                background-color: rgba(0, 0, 0, 180);
                color: white;
                padding: 3px 8px;
                border-radius: 5px;
                font-size: 12px;
                font-weight: bold;
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

void MainWindow::sendModeChangeRequest(const QString &mode, const CameraInfo &camera)
{
    if (camera.ip.isEmpty()) {
        qWarning() << "[모드 변경] 카메라 IP 없음 →" << camera.name;
        return;
    }

    if (!socketMap.contains(camera.ip)) {
        qWarning() << "[모드 변경] WebSocket 연결 없음 →" << camera.name;
        return;
    }

    QWebSocket *socket = socketMap[camera.ip];
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[모드 변경] WebSocket 비연결 상태 →" << camera.name;
        return;
    }

    // WebSocket 메시지 생성
    QJsonObject payload;
    payload["type"] = "set_mode";
    payload["mode"] = mode;

    QJsonDocument doc(payload);
    QString message = doc.toJson(QJsonDocument::Compact);
    socket->sendTextMessage(message);

    qDebug() << "[WebSocket] 모드 변경 메시지 전송됨:" << message;

    // 응답 처리 (임시 슬롯 연결, 필요 시 해제 고려)
    connect(socket, &QWebSocket::textMessageReceived, this, [=](const QString &msg) {
        QJsonDocument respDoc = QJsonDocument::fromJson(msg.toUtf8());
        if (!respDoc.isObject()) return;

        QJsonObject obj = respDoc.object();
        QString type = obj["type"].toString();

        if (type == "mode_change_ack") {
            QString status = obj["status"].toString();
            QString serverMessage = obj["message"].toString();

            if (status == "error") {
                qWarning() << "[모드 변경 실패]" << serverMessage;
                QMessageBox::warning(this, "모드 변경 실패", serverMessage);
            } else {
                qDebug() << "[모드 변경 성공 응답]" << serverMessage;
            }
        }
    });
}

void MainWindow::setupWebSocketConnections()
{
    for (const CameraInfo &camera : cameraList) {
        if (socketMap.contains(camera.ip)) continue;  // 이미 연결된 경우 생략

        QWebSocket *socket = new QWebSocket();

        connect(socket, &QWebSocket::sslErrors, this, [socket](const QList<QSslError> &) {
            socket->ignoreSslErrors();
        });

        connect(socket, &QWebSocket::connected, this, &MainWindow::onSocketConnected);
        connect(socket, &QWebSocket::disconnected, this, &MainWindow::onSocketDisconnected);
        connect(socket, &QWebSocket::errorOccurred,
                this, &MainWindow::onSocketErrorOccurred);
        connect(socket, &QWebSocket::textMessageReceived,
                this, &MainWindow::onSocketMessageReceived);

        QString wsUrl = QString("wss://%1:8443/ws").arg(camera.ip);
        socket->open(QUrl(wsUrl));
        socketMap[camera.ip] = socket;
    }
}

void MainWindow::onSocketConnected() {
    qDebug() << "[WebSocket] 연결됨";
}

void MainWindow::onSocketDisconnected() {
    qDebug() << "[WebSocket] 연결 해제됨";
}

void MainWindow::onSocketErrorOccurred(QAbstractSocket::SocketError error) {
    qWarning() << "[WebSocket] 에러 발생:" << error;
}

void MainWindow::onSocketMessageReceived(const QString &message) {
    qDebug() << "[WebSocket] 메시지 수신:" << message;
}
