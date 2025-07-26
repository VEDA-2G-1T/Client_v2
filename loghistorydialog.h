#ifndef LOGHISTORYDIALOG_H
#define LOGHISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QVector>
#include <QLabel>
#include <QNetworkAccessManager>
#include "logentry.h"  // ✅ 이 줄 꼭 필요함!

class LogHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogHistoryDialog(const QVector<LogEntry> &logs, QWidget *parent = nullptr);

private:
    void setupUI();
    void populateTable(const QVector<LogEntry> &logs);

    QTableWidget *logTable;
    QLabel *imagePreviewLabel;
    QNetworkAccessManager *previewManager;
};

#endif // LOGHISTORYDIALOG_H
