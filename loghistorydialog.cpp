#include "loghistorydialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QPixmap>
#include <QNetworkReply>
#include <QUrl>

LogHistoryDialog::LogHistoryDialog(const QVector<LogEntry> &logs, QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    populateTable(logs);

    setWindowTitle("전체 로그 보기");
    setFixedSize(1000, 600);  // ➕ 너비 확장
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
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // 좌측 로그 테이블 + 버튼
    QVBoxLayout *leftLayout = new QVBoxLayout();

    QLabel *title = new QLabel("📜 Event Search");
    leftLayout->addWidget(title);

    logTable = new QTableWidget(this);
    logTable->setColumnCount(5);  // imageUrl 포함
    logTable->setHorizontalHeaderLabels({"Time", "Camera", "Function", "Event", "ImageURL"});
    logTable->setColumnHidden(4, true);  // URL은 숨김

    // ✅ 열 너비 개별 조정
    logTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    logTable->setColumnWidth(0, 200);  // Time
    logTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    logTable->setColumnWidth(1, 100);  // Camera
    logTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    logTable->setColumnWidth(2, 100);  // Function
    logTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);  // Event

    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    logTable->setShowGrid(false);

    leftLayout->addWidget(logTable);

    QPushButton *closeButton = new QPushButton("닫기");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    leftLayout->addWidget(closeButton, 0, Qt::AlignRight);

    mainLayout->addLayout(leftLayout, 2);  // 왼쪽은 비율 2

    // 우측 이미지 미리보기
    imagePreviewLabel = new QLabel("🖼️ 로그를 선택하세요");
    imagePreviewLabel->setFixedSize(400, 300);
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setStyleSheet("background-color: #1e1e1e; border: 1px solid #555;");
    mainLayout->addWidget(imagePreviewLabel, 1);  // 오른쪽은 비율 1

    // 이미지 다운로드 핸들러
    previewManager = new QNetworkAccessManager(this);

    connect(logTable, &QTableWidget::cellClicked, this, [=](int row, int) {
        QString url = logTable->item(row, 4)->text().trimmed();
        if (url.isEmpty()) {
            imagePreviewLabel->setText("❌ 이미지 없음");
            imagePreviewLabel->setPixmap(QPixmap());
            return;
        }

        QNetworkRequest req{QUrl(url)};
        QNetworkReply *reply = previewManager->get(req);
        connect(reply, &QNetworkReply::finished, this, [=]() {
            QPixmap pix;
            pix.loadFromData(reply->readAll());
            if (!pix.isNull()) {
                imagePreviewLabel->setPixmap(pix.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                imagePreviewLabel->setText("❌ 이미지 로드 실패");
                imagePreviewLabel->setPixmap(QPixmap());
            }
            reply->deleteLater();
        });
    });
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
        logTable->setItem(row, 4, new QTableWidgetItem(entry.imageUrl));  // ✅ 숨겨진 이미지 URL
        ++row;
    }
}
