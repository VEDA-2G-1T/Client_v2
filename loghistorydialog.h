#ifndef LOGHISTORYDIALOG_H
#define LOGHISTORYDIALOG_H

#include "logentry.h"       // 로그 데이터 구조체
#include "imageenhancer.h"  // 이미지 향상 기능

#include <QDialog>
#include <QTableWidget>
#include <QVector>
#include <QLabel>
#include <QTabWidget>
#include <QCheckBox>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QMouseEvent>

class LogHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogHistoryDialog(const QVector<LogEntry> &logs, QWidget *parent = nullptr);

protected:
    // ✅ Frameless 드래그 이동 지원
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // ✅ 내부 UI 구성 및 동작 함수들
    void setupUI();
    void populateTabs();                 // 카메라별 탭 구성
    void applyFilter();                  // 체크박스 필터링 적용
    void handleRowClick(int row, int col); // 로그 클릭 시 이미지 표시

    // ✅ 로그 데이터
    QVector<LogEntry> allLogs;           // 전체 로그 보관
    QTabWidget *tabWidget;               // 카메라별 탭 위젯

    // ✅ 필터 체크박스
    QCheckBox *totalCheck;
    QCheckBox *blurCheck;
    QCheckBox *ppeCheck;
    QCheckBox *trespassCheck;
    QCheckBox *fallCheck;

    // ✅ 우측 이미지 미리보기
    QLabel *imagePreviewLabel;
    QPixmap originalPreviewPix;          // 원본 이미지 저장
    QNetworkAccessManager *previewManager; // 네트워크 이미지 요청용

    // ✅ Frameless 이동 제어
    bool dragging = false;
    QPoint dragPosition;
};

#endif // LOGHISTORYDIALOG_H
