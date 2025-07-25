#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cameraregistrationdialog.h"

#include <QMainWindow>
#include <QListWidget>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QDateTime>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private:
    // 🔲 상단 바
    void setupTopBar();
    QWidget *topBar;
    QLabel *timeLabel;

    // 📂 좌측 카메라 목록
    void setupCameraList();
    QListWidget *cameraListPanel;
    QWidget *cameraListWrapper;

    // 🖼 중앙 영상 타일
    void setupVideoGrid();
    QWidget *videoGridPanel;

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
