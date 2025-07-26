#include "mainwindow.h"
#include "camerainfo.h"
#include "cameraitemwidget.h"
#include "cameraregistrationdialog.h"
#include "logitemwidget.h"

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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonArray>
#include <algorithm>
#include <QFontDatabase>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Smart SafetyNet");
    resize(1460, 720);

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
    bodyLayout->addWidget(eventLogScroll, 2);

    mainLayout->addLayout(bodyLayout);
    setCentralWidget(central);

    networkManager = new QNetworkAccessManager(this);  // ✅ 초기화

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

    int idB = QFontDatabase::addApplicationFont(":/resources/fonts/01HanwhaB.ttf");
    int idR = QFontDatabase::addApplicationFont(":/resources/fonts/02HanwhaR.ttf");
    int idL = QFontDatabase::addApplicationFont(":/resources/fonts/03HanwhaL.ttf");
    int gidL = QFontDatabase::addApplicationFont(":/resources/fonts/06HanwhaGothicL.ttf");

    QString fontB = QFontDatabase::applicationFontFamilies(idB).at(0);
    QString fontR = QFontDatabase::applicationFontFamilies(idR).at(0);
    QString fontL = QFontDatabase::applicationFontFamilies(idL).at(0);
    QString gfontL = QFontDatabase::applicationFontFamilies(gidL).at(0);

    QLabel *logoLabel = new QLabel("Smart SafetyNet");
    logoLabel->setFont(QFont(fontB, 20));  // 굵은 타이틀
    logoLabel->setStyleSheet("color: #f37321;");

    timeLabel = new QLabel();
    timeLabel->setFont(QFont(gfontL, 10));
    timeLabel->setStyleSheet("color: white;");

    QPushButton *closeButton = new QPushButton("종료");
    closeButton->setFixedSize(50, 30);  // 너비 60px, 높이 36px
    closeButton->setFont(QFont(gfontL, 8));
    closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #222;
            color: #f37321;
            border: 1px solid #444;
            border-radius: 4px;
            padding: 4px 10px;
        }
    )");

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
                loadInitialLogs();

            }
        });

        connect(healthButton, &QPushButton::clicked, this, &MainWindow::performHealthCheck);

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
                                static int id = QFontDatabase::addApplicationFont(":/resources/fonts/02HanwhaR.ttf");
                                static QString fontR = QFontDatabase::applicationFontFamilies(id).at(0);
                                QFont cameraFont(fontR, 14);

                                QLabel *placeholder = new QLabel(QString("CAMERA #%1").arg(row * 3 + col + 1));
                                placeholder->setFont(cameraFont);  // ✅ 여기에 추가!

                                placeholder->setObjectName("placeholder");
                                placeholder->setAlignment(Qt::AlignCenter);
                                placeholder->setMinimumSize(320, 240);
                                placeholder->setStyleSheet(R"(
                                    background-color: rgba(0, 0, 0, 180);
                                    color: white;
                                    border-radius: 5px;
                                    font-size: 14px;
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

    // 🔹 폰트 등록 (반복되지 않도록 static으로)
    static int id = QFontDatabase::addApplicationFont(":/resources/fonts/02HanwhaR.ttf");
    static QString fontR = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont cameraFont(fontR, 14);  // 크기 원하는 대로 조절

    const QSize tileMinSize(320, 240);
    for (int row : {0, 1, 2}) {
        for (int col : {0, 1, 2}) {
            if ((row == 1 && col == 1) || (row == 1 && col == 2) ||
                (row == 2 && col == 1) || (row == 2 && col == 2))
                continue;

            QString labelText = cameraLabels.value({row, col}, QString("(%1,%2)").arg(row).arg(col));
            QLabel *tile = new QLabel(labelText);
            tile->setFont(cameraFont);
            tile->setObjectName("placeholder");  // ✅ 자리 표시자임을 명시
            tile->setAutoFillBackground(true);
            tile->setStyleSheet(R"(
                background-color: rgba(0, 0, 0, 180);
                color: white;
                padding: 3px 8px;
                border-radius: 5px;
                font-size: 14px;
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
    // 전체 로그 영역 래퍼
    eventLogPanelWrapper = new QWidget();
    eventLogPanelWrapper->setStyleSheet("background-color: #1e1e1e;");
    eventLogPanelWrapper->setMinimumHeight(0);
    eventLogPanelWrapper->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    // 👉 외부 레이아웃 (상단 버튼 + 하단 로그)
    QVBoxLayout *outerLayout = new QVBoxLayout(eventLogPanelWrapper);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // ✅ 상단 버튼 영역
    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);   // 좌우 여백 제거
    headerLayout->setSpacing(0);                    // 버튼 간 여백 제거

    viewAllLogsButton = new QPushButton();
    viewAllLogsButton->setIcon(QIcon(":/resources/icons/search.png"));  // 리소스 등록 필요
    viewAllLogsButton->setIconSize(QSize(28, 28));  // 적당히 조절 가능
    viewAllLogsButton->setCursor(Qt::PointingHandCursor);
    viewAllLogsButton->setToolTip("전체 로그 보기");
    viewAllLogsButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
            padding: 10px 0px;
        }
        QPushButton:hover {
            background-color: #f37321;
        }
    )");

    connect(viewAllLogsButton, &QPushButton::clicked, this, [=]() {
        LogHistoryDialog *dialog = new LogHistoryDialog(logEntries, this);  // ✅ 로그 전달
        dialog->exec();
    });

    headerLayout->addWidget(viewAllLogsButton);  // ✅ 중앙 정렬 제거 → 전체 폭 사용
    headerWidget->setLayout(headerLayout);
    outerLayout->addWidget(headerWidget);        // ✅ 상단에 고정

    // ✅ 로그 항목 영역
    QWidget *logContainer = new QWidget();
    eventLogLayout = new QVBoxLayout(logContainer);
    eventLogLayout->setContentsMargins(0, 0, 0, 0);
    eventLogLayout->setSpacing(0);

    outerLayout->addWidget(logContainer);
    outerLayout->addStretch();  // 남는 공간 채움

    // 스크롤 설정
    eventLogScroll = new QScrollArea();
    eventLogScroll->setWidgetResizable(true);
    eventLogScroll->setWidget(eventLogPanelWrapper);
    eventLogScroll->setFixedWidth(200);
    eventLogScroll->setAlignment(Qt::AlignTop);

    eventLogScroll->setStyleSheet(R"(
        QScrollArea {
            background-color: #1e1e1e;
            border: none;
        }
        QWidget {
            background-color: #1e1e1e;
        }
    )");
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
}

void MainWindow::setupWebSocketConnections()
{
    for (const CameraInfo &camera : cameraList) {
        // 이미 연결된 경우 생략
        if (socketMap.contains(camera.ip)) {
            QWebSocket *existing = socketMap[camera.ip];
            if (existing && existing->state() == QAbstractSocket::ConnectedState) {
                continue;
            }
        }

        QWebSocket *socket = new QWebSocket();

        connect(socket, &QWebSocket::sslErrors, this, [socket](const QList<QSslError> &) {
            socket->ignoreSslErrors();
        });

        connect(socket, &QWebSocket::connected, this, [=]() {
            qDebug() << "[WebSocket 연결 성공]" << camera.ip;
            socketMap[camera.ip] = socket;  // ✅ 연결 성공 후에 등록

            // 초기 헬시 상태 설정할 수도 있음
            for (int i = 0; i < listLayout->count(); ++i) {
                if (CameraItemWidget *w = qobject_cast<CameraItemWidget *>(listLayout->itemAt(i)->widget())) {
                    if (w->getCameraInfo().ip == camera.ip) {
                        w->updateHealthStatus("🔗 연결됨", "lightblue");
                        break;
                    }
                }
            }
        });

        connect(socket, &QWebSocket::disconnected, this, [=]() {
            qDebug() << "[WebSocket 연결 해제]" << camera.ip;
            socket->deleteLater();
            socketMap.remove(camera.ip);

            for (int i = 0; i < listLayout->count(); ++i) {
                if (CameraItemWidget *w = qobject_cast<CameraItemWidget *>(listLayout->itemAt(i)->widget())) {
                    if (w->getCameraInfo().ip == camera.ip) {
                        w->updateHealthStatus("❌ 미연결", "orange");
                        break;
                    }
                }
            }
        });

        connect(socket, &QWebSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error) {
            qWarning() << "[WebSocket 에러]" << camera.ip << error;
            // 연결 에러가 발생해도 연결 시도는 하되, 상태 업데이트
            for (int i = 0; i < listLayout->count(); ++i) {
                if (CameraItemWidget *w = qobject_cast<CameraItemWidget *>(listLayout->itemAt(i)->widget())) {
                    if (w->getCameraInfo().ip == camera.ip) {
                        w->updateHealthStatus("❌ 연결 실패", "red");
                        break;
                    }
                }
            }
        });

        connect(socket, &QWebSocket::textMessageReceived,
                this, &MainWindow::onSocketMessageReceived);

        QString wsUrl = QString("wss://%1:8443/ws").arg(camera.ip);
        socket->open(QUrl(wsUrl));  // ✅ 연결 시도는 하지만, 등록은 성공 후에만
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

void MainWindow::onSocketMessageReceived(const QString &message)
{
    qDebug() << "[WebSocket 수신 메시지]" << message;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "[WebSocket 메시지] JSON 파싱 실패";
        return;
    }

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    QJsonObject data = obj["data"].toObject();

    qDebug() << "📨 [WebSocket 타입]" << type;

    QString ipSender;
    QWebSocket *senderSocket = qobject_cast<QWebSocket*>(sender());
    for (auto it = socketMap.begin(); it != socketMap.end(); ++it) {
        if (it.value() == senderSocket) {
            ipSender = it.key();
            break;
        }
    }

    if (ipSender.isEmpty()) {
        qWarning() << "[WebSocket] 발신자 IP 찾기 실패";
        return;
    }

    const CameraInfo *cameraPtr = nullptr;
    for (int i = 0; i < cameraList.size(); ++i) {
        if (cameraList[i].ip.trimmed() == ipSender.trimmed()) {
            cameraPtr = &cameraList[i];
            break;
        }
    }

    if (!cameraPtr) {
        qWarning() << "[WebSocket] CameraInfo 찾기 실패 for IP:" << ipSender;
        return;
    }
    const CameraInfo &camera = *cameraPtr;

    if (type == "new_detection") {
        int person = data["person_count"].toInt();
        int helmet = data["helmet_count"].toInt();
        int vest = data["safety_vest_count"].toInt();
        double conf = data["avg_confidence"].toDouble();
        QString imagePath = data["image_path"].toString();
        QString ts = data["timestamp"].toString();

        QString event;
        QString details = QString("👷 %1명 | ⛑️ %2명 | 🦺 %3명 | 신뢰도: %4")
                              .arg(person).arg(helmet).arg(vest).arg(conf, 0, 'f', 2);

        if (helmet < person && vest >= person)
            event = "⛑️ 헬멧 미착용 감지";
        else if (vest < person && helmet >= person)
            event = "🦺 조끼 미착용 감지";
        else
            event = "⛑️ 🦺 PPE 미착용 감지";

        qDebug() << "[PPE 이벤트]" << event << "IP:" << camera.ip;

        if (event.contains("미착용")) {
            int count = ppeViolationStreakMap[camera.name] + 1;
            ppeViolationStreakMap[camera.name] = count;

            if (count >= 4) {
                QDialog *popup = new QDialog(this);
                popup->setWindowTitle("지속적인 PPE 위반");
                popup->setModal(false);
                popup->setStyleSheet(R"(
                    QDialog {
                        background-color: #2b2b2b;
                        color: white;
                    }
                    QLabel {
                        color: white;
                        font-size: 13px;
                    }
                    QPushButton {
                        background-color: #444;
                        color: white;
                        border: 1px solid #666;
                        border-radius: 4px;
                        padding: 4px 12px;
                        font-size: 12px;
                    }
                    QPushButton:hover {
                        background-color: #666;
                    }
                )");

                QVBoxLayout *layout = new QVBoxLayout(popup);

                // 텍스트 메시지
                QLabel *textLabel = new QLabel(QString("⚠️ <b>%1</b> 카메라에서<br>PPE 미착용이 <b>연속 4회</b> 감지되었습니다!").arg(camera.name));
                textLabel->setTextFormat(Qt::RichText);
                textLabel->setWordWrap(true);
                layout->addWidget(textLabel);

                // 이미지가 있을 경우 비동기 로딩
                if (!imagePath.isEmpty()) {
                    QString cleanPath = imagePath;
                    if (cleanPath.startsWith("../"))
                        cleanPath = cleanPath.mid(3);
                    QString urlStr = QString("http://%1/%2").arg(camera.ip, cleanPath);
                    QUrl url(urlStr);
                    QNetworkRequest request(url);

                    QNetworkAccessManager *manager = new QNetworkAccessManager(popup);
                    QNetworkReply *reply = manager->get(request);

                    connect(reply, &QNetworkReply::finished, popup, [=]() {
                        reply->deleteLater();
                        QPixmap pix;
                        pix.loadFromData(reply->readAll());
                        if (!pix.isNull()) {
                            QLabel *imgLabel = new QLabel();
                            imgLabel->setPixmap(pix.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                            layout->addWidget(imgLabel);
                            popup->adjustSize();
                        }
                    });
                }

                // 확인 버튼
                QPushButton *okBtn = new QPushButton("확인");
                connect(okBtn, &QPushButton::clicked, popup, &QDialog::accept);
                layout->addWidget(okBtn, 0, Qt::AlignRight);

                popup->show();

                // streak 리셋
                ppeViolationStreakMap[camera.name] = 0;
            }
        } else {
            ppeViolationStreakMap[camera.name] = 0;
        }

        addLogEntry(camera.name, "PPE", event, imagePath, details, camera.ip, ts);
    }

    else if (type == "new_trespass") {
        QString ts = data["timestamp"].toString();
        int count = data["count"].toInt();
        QString imagePath = data["image_path"].toString();  // ✅ 추가

        if (count > 0) {
            QString event = QString("🚷 무단 침입 감지 (%1명)").arg(count);
            QString details = QString("감지 시각: %1 | 침입자 수: %2").arg(ts).arg(count);
            addLogEntry(camera.name, "Night", event, imagePath, details, camera.ip, ts);  // ✅ 이미지 포함
        }
    }

    else if (type == "new_blur") {
        QString ts = data["timestamp"].toString();
        QString key = camera.name + "_" + ts;
        if (recentBlurLogKeys.contains(key)) {
            qDebug() << "[BLUR 중복 무시]" << key;
            return;
        }

        int count = data["count"].toInt();
        QString event = QString("🔍 %1명 감지").arg(count);
        addLogEntry(camera.name, "Blur", event, "", "", camera.ip, ts);
        recentBlurLogKeys.insert(key);
    }

    else if (type == "anomaly_status") {
        QString status = data["status"].toString();
        QString timestamp = data["timestamp"].toString();

        qDebug() << "[이상소음 상태]" << status << "at" << timestamp;

        if (status == "detected" && lastAnomalyStatus[camera.name] != "detected") {
            addLogEntry(camera.name, "Sound", "⚠️ 이상소음 감지됨", "", "이상소음 발생", camera.ip, timestamp);
        }
        else if (status == "cleared" && lastAnomalyStatus[camera.name] == "detected") {
            addLogEntry(camera.name, "Sound", "✅ 이상소음 해제됨", "", "이상소음 정상 상태", camera.ip, timestamp);
        }

        lastAnomalyStatus[camera.name] = status;
    }

    else if (type == "new_fall") {
        QString ts = data["timestamp"].toString();
        int count = data["count"].toInt();

        if (count > 0) {
            QString event = "🚨 낙상 감지";
            QString details = QString("낙상 감지 시각: %1").arg(ts);
            addLogEntry(camera.name, "Fall", event, "", details, camera.ip, ts);
        }
    }

    else if (type == "stm_status_update") {
        double temp = data["temperature"].toDouble();
        int light = data["light"].toInt();
        bool buzzer = data["buzzer_on"].toBool();
        bool led = data["led_on"].toBool();

        QString details = QString("🌡️ 온도: %1°C | 💡 밝기: %2 | 🔔 버저: %3 | 💡 LED: %4")
                              .arg(temp, 0, 'f', 2)
                              .arg(light)
                              .arg(buzzer ? "ON" : "OFF")
                              .arg(led ? "ON" : "OFF");

        healthCheckResponded.insert(camera.ip);

        // ✅ 여기 추가해야 드롭다운 옆에 "✅ 정상"이 뜸!
        for (int i = 0; i < listLayout->count(); ++i) {
            QLayoutItem *item = listLayout->itemAt(i);
            if (CameraItemWidget *w = qobject_cast<CameraItemWidget *>(item->widget())) {
                if (w->getCameraInfo().ip == camera.ip) {
                    w->updateHealthStatus("✅ 센서 연걸 상태 정상", "lightgreen");
                    break;
                }
            }
        }
    }


    else if (type == "mode_change_ack") {
        QString status = obj["status"].toString();
        QString mode = obj["mode"].toString();
        QString message = obj["message"].toString();

        if (status == "error") {
            qWarning() << "[모드 변경 실패]" << message;
            QMessageBox::warning(this, "모드 변경 실패", message);
        } else {
            qDebug() << "[모드 변경 성공 응답]" << mode;
        }
    }

    else if (type == "log") {
        QString event = data["event"].toString();
        QString details = data["details"].toString();
        QString function = data["function"].toString();  // 예: "Blur", "PPE" 등
        QString imagePath = data["image_path"].toString();
        QString ts = data["timestamp"].toString();

        addLogEntry(camera.name, function, event, imagePath, details, camera.ip);
    }

    else {
        qWarning() << "[WebSocket] 알 수 없는 타입 수신:" << type;
    }
}

void MainWindow::addLogEntry(const QString &cameraName,
                             const QString &function,
                             const QString &event,
                             const QString &imagePath,
                             const QString &details,
                             const QString &ip,
                             const QString &timestamp)  // ✅ 추가
{
    QString time = timestamp;  // ✅ 이제 클라이언트 시간 말고 서버 시간 사용
    QString imageUrl;

    if (!imagePath.isEmpty()) {
        QString cleanPath = imagePath.startsWith("../") ? imagePath.mid(3) : imagePath;
        imageUrl = QString("http://%1/%2").arg(ip, cleanPath);
    }

    logEntries.insert(0, {cameraName, function, event, time, imageUrl});

    LogItemWidget *logItem = new LogItemWidget(cameraName, event, time, imageUrl);
    eventLogLayout->insertWidget(0, logItem);

    if (eventLogLayout->count() > 100) {
        QLayoutItem *oldItem = eventLogLayout->takeAt(eventLogLayout->count() - 1);
        if (oldItem && oldItem->widget()) delete oldItem->widget();
    }
}

void MainWindow::loadInitialLogs()
{
    logEntries.clear();  // 초기화

    int totalRequests = cameraList.size() * 2;  // PPE + Trespass
    int *completedCount = new int(0);  // 람다에서 사용 가능하도록 동적 할당

    // ✅ std::function으로 정의해야 const lambda 안에서도 호출 가능
    std::function<void()> trySortAndPrint;

    trySortAndPrint = [=]() {
        (*completedCount)++;
        if (*completedCount == totalRequests) {
            qDebug() << "[모든 초기 로그 수신 완료] 총" << logEntries.size() << "건";

            std::sort(logEntries.begin(), logEntries.end(), [](const LogEntry &a, const LogEntry &b) {
                return QDateTime::fromString(a.timestamp, "yyyy-MM-dd HH:mm:ss") >
                       QDateTime::fromString(b.timestamp, "yyyy-MM-dd HH:mm:ss");
            });

            delete completedCount;  // 누수 방지
        }
    };

    for (const CameraInfo &camera : cameraList) {
        // ✅ PPE 요청
        QString urlPPE = QString("https://%1:8443/api/detections").arg(camera.ip);
        QNetworkRequest reqPPE{QUrl(urlPPE)};
        QNetworkReply *replyPPE = networkManager->get(reqPPE);
        replyPPE->ignoreSslErrors();

        connect(replyPPE, &QNetworkReply::finished, this, [=]() {
            replyPPE->deleteLater();
            if (replyPPE->error() != QNetworkReply::NoError) return trySortAndPrint();

            QJsonDocument doc = QJsonDocument::fromJson(replyPPE->readAll());
            if (!doc.isObject()) return trySortAndPrint();

            QJsonArray arr = doc["detections"].toArray();
            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();
                QString ts = obj["timestamp"].toString();
                int person = obj["person_count"].toInt();
                int helmet = obj["helmet_count"].toInt();
                int vest = obj["safety_vest_count"].toInt();
                double conf = obj["avg_confidence"].toDouble();
                QString imgPath = obj["image_path"].toString();

                QString event;
                if (helmet < person && vest >= person)
                    event = "⛑️ 헬멧 미착용 감지";
                else if (vest < person && helmet >= person)
                    event = "🦺 조끼 미착용 감지";
                else
                    event = "⛑️ 🦺 PPE 미착용 감지";

                QString imageUrl;
                if (!imgPath.isEmpty()) {
                    QString cleanPath = imgPath.startsWith("../") ? imgPath.mid(3) : imgPath;
                    imageUrl = QString("http://%1/%2").arg(camera.ip, cleanPath);
                }

                logEntries.append({camera.name, "PPE", event, ts, imageUrl});
            }

            trySortAndPrint();
        });

        // ✅ 무단 침입 요청
        QString urlTrespass = QString("https://%1:8443/api/trespass").arg(camera.ip);
        QNetworkRequest reqTrespass{QUrl(urlTrespass)};
        QNetworkReply *replyTrespass = networkManager->get(reqTrespass);
        replyTrespass->ignoreSslErrors();

        connect(replyTrespass, &QNetworkReply::finished, this, [=]() {
            replyTrespass->deleteLater();
            if (replyTrespass->error() != QNetworkReply::NoError) return trySortAndPrint();

            QByteArray raw = replyTrespass->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(raw);
            if (!doc.isObject()) return trySortAndPrint();

            QJsonArray arr = doc["detections"].toArray();
            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();
                QString ts = obj["timestamp"].toString();
                int count = obj["count"].toInt();
                QString imgPath = obj["image_path"].toString();

                QString event = QString("🚷 무단 침입 감지 (%1명)").arg(count);
                QString imageUrl;
                if (!imgPath.isEmpty()) {
                    QString cleanPath = imgPath.startsWith("../") ? imgPath.mid(3) : imgPath;
                    imageUrl = QString("https://%1:8443/%2").arg(camera.ip, cleanPath);
                }

                logEntries.append({camera.name, "Night", event, ts, imageUrl});
            }

            trySortAndPrint();
        });
    }
}

void MainWindow::performHealthCheck()
{
    // 🔄 이전 응답 기록 초기화
    healthCheckResponded.clear();

    for (const CameraInfo &camera : cameraList) {
        // ✅ 연결 상태까지 확인
        if (socketMap.contains(camera.ip) &&
            socketMap[camera.ip]->state() == QAbstractSocket::ConnectedState) {

            QWebSocket *socket = socketMap[camera.ip];

            // ✅ 헬시체크 요청 전송
            QJsonObject req;
            req["type"] = "request_stm_status";
            QJsonDocument doc(req);
            socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));

            qDebug() << "[헬시 체크 요청 전송됨]" << camera.ip;

            // ✅ ⏳ '확인 중' 표시
            for (int i = 0; i < listLayout->count(); ++i) {
                if (CameraItemWidget *w = qobject_cast<CameraItemWidget *>(listLayout->itemAt(i)->widget())) {
                    if (w->getCameraInfo().ip == camera.ip) {
                        w->updateHealthStatus("⏳ 확인 중", "gray");
                        break;
                    }
                }
            }

            // ✅ 5초 내 응답 없으면 경고 상태로 업데이트
            QTimer::singleShot(5000, this, [=]() {
                if (!healthCheckResponded.contains(camera.ip)) {
                    for (int i = 0; i < listLayout->count(); ++i) {
                        if (CameraItemWidget *w = qobject_cast<CameraItemWidget *>(listLayout->itemAt(i)->widget())) {
                            if (w->getCameraInfo().ip == camera.ip) {
                                w->updateHealthStatus("⚠️ 센서 상태를 점검하세요", "#f37321");
                                break;
                            }
                        }
                    }
                }
            });

        } else {
            // ❌ WebSocket 미연결 상태 → 아무 것도 안 함 (setupWebSocketConnections()에서 이미 표시됨)
            qDebug() << "[헬시 체크 스킵] 연결 안 된 카메라:" << camera.ip;
        }
    }
}
