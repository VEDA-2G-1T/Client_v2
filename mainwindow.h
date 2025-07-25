#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cameraregistrationdialog.h"
#include "camerainfo.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsTextItem>
#include <QtWebSockets/QWebSocket>
#include <QMap>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:

    // 상단 바
    void setupTopBar();
    QWidget *topBar;
    QLabel *timeLabel;

    QPair<int, int> findEmptyVideoSlot();

    // 좌측 카메라 목록
    void setupCameraList();
    void refreshCameraListItems();           // 새로 추가: 리스트 아이템 갱신 함수
    QWidget *cameraListWrapper = nullptr;    // 멤버 변수로 유지
    QVBoxLayout *wrapperLayout = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *listContainer = nullptr;
    QVBoxLayout *listLayout = nullptr;
    QVector<CameraInfo> cameraList;

    // 중앙 영상 타일
    void setupVideoGrid();
    QWidget *videoGridPanel;

    QVector<QMediaPlayer*> players;
    QVector<QVideoWidget*> videoWidgets;

    // 우측 이벤트 로그
    void setupEventLog();
    QTableWidget *eventLogPanel;

    QMap<QString, QWebSocket*> socketMap;    // IP 주소 → QWebSocket 포인터 매핑
    void setupWebSocketConnections();


private slots:
    void sendModeChangeRequest(const QString &mode, const CameraInfo &camera);
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketErrorOccurred(QAbstractSocket::SocketError error);
    void onSocketMessageReceived(const QString &message);
};

#endif // MAINWINDOW_H
