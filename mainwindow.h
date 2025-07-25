#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cameraregistrationdialog.h"

#include <QMainWindow>
#include <QListWidget>
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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

protected:
    // ✅ 이벤트 필터 오버라이드 선언
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // 🔲 상단 바
    void setupTopBar();
    QWidget *topBar;
    QLabel *timeLabel;

    QPair<int, int> findEmptyVideoSlot();

    // 📂 좌측 카메라 목록
    void setupCameraList();
    QListWidget *cameraListPanel;
    QWidget *cameraListWrapper;

    // 🖼 중앙 영상 타일
    void setupVideoGrid();
    QWidget *videoGridPanel;

    QVector<QMediaPlayer*> players;
    QVector<QVideoWidget*> videoWidgets;

    // 📋 우측 이벤트 로그
    void setupEventLog();
    QTableWidget *eventLogPanel;
};

#endif // MAINWINDOW_H

/*
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;
};

#endif // MAINWINDOW_H
*/
