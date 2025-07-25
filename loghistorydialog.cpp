#include "loghistorydialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>

LogHistoryDialog::LogHistoryDialog(const QVector<LogEntry> &logs, QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    populateTable(logs);

    setWindowTitle("전체 로그 보기");
    setFixedSize(800, 600);
    setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: white;
        }
        QLabel {
            font-size: 16px;
            font-weight: bold;
            color: orange;
        }
        QHeaderView::section {
            background-color: #2b2b2b;
            color: white;
            font-weight: bold;
            border-bottom: 1px solid #555;
        }
        QTableWidget {
            background-color: #1e1e1e;
            color: white;
            gridline-color: #444;
        }
    )");
}

void LogHistoryDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *title = new QLabel("📜 Event Search");
    mainLayout->addWidget(title);

    logTable = new QTableWidget(this);
    logTable->setColumnCount(4);
    logTable->setHorizontalHeaderLabels({"Time", "Camera", "Function", "Event"});

    // ✅ 열 너비 개별 조정
    logTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    logTable->setColumnWidth(0, 200);  // Time

    logTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    logTable->setColumnWidth(1, 100);  // Camera

    logTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    logTable->setColumnWidth(2, 100);  // Function

    logTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);  // Event
    logTable->setColumnWidth(3, 100);  // Event

    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    logTable->setShowGrid(false);

    mainLayout->addWidget(logTable);

    QPushButton *closeButton = new QPushButton("닫기");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeButton, 0, Qt::AlignRight);
}

void LogHistoryDialog::populateTable(const QVector<LogEntry> &logs)
{
    logTable->setRowCount(logs.size());
    int row = 0;
    for (const LogEntry &entry : logs) {
        logTable->setItem(row, 0, new QTableWidgetItem(entry.timestamp));
        logTable->setItem(row, 1, new QTableWidgetItem(entry.cameraName));
        logTable->setItem(row, 2, new QTableWidgetItem(entry.function));
        logTable->setItem(row, 3, new QTableWidgetItem(entry.event));
        ++row;
    }
}
