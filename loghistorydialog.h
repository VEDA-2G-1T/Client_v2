#ifndef LOGHISTORYDIALOG_H
#define LOGHISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QVector>
#include <QLabel>
#include <QTabWidget>
#include <QCheckBox>
#include <QNetworkAccessManager>
#include "logentry.h"  // ✅ 꼭 필요함!

class LogHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogHistoryDialog(const QVector<LogEntry> &logs, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUI();
    void populateTabs();         // 탭 구성 함수
    void applyFilter();          // 체크박스 & 탭 필터링 적용
    void handleRowClick(int row, int col);  // 이미지 클릭 핸들러

    QVector<LogEntry> allLogs;   // 전체 로그

    QTabWidget *tabWidget;       // 카메라별 탭

    QCheckBox *totalCheck;
    QCheckBox *blurCheck;
    QCheckBox *ppeCheck;
    QCheckBox *trespassCheck;
    QCheckBox *fallCheck;

    QLabel *imagePreviewLabel;   // 우측 이미지 미리보기
    QNetworkAccessManager *previewManager;  // 이미지 요청용

    bool dragging = false;
    QPoint dragPosition;
};

#endif // LOGHISTORYDIALOG_H
