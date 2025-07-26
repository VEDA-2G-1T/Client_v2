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
    setFixedSize(1000, 600);
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
    // 🔹 전체를 감싸는 수직 레이아웃
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(5, 5, 0, 0);
    outerLayout->setSpacing(5);

    // 🔹 상단: 제목 + 닫기 버튼 (수평)
    QHBoxLayout *topLayout = new QHBoxLayout();
    QLabel *title = new QLabel("Event Log History");
    QPushButton *closeButton = new QPushButton("닫기");
    closeButton->setFixedSize(60, 28);
    topLayout->addWidget(title);
    topLayout->addStretch();
    topLayout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // 🔸 하단: 본문 (로그 테이블 + 이미지)
    QHBoxLayout *contentLayout = new QHBoxLayout();

    // 좌측 로그 테이블
    QVBoxLayout *leftLayout = new QVBoxLayout();
    logTable = new QTableWidget(this);
    logTable->setColumnCount(5);
    logTable->setHorizontalHeaderLabels({"Time", "Camera", "Function", "Event", "ImageURL"});
    logTable->setColumnHidden(4, true);

    logTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    logTable->setColumnWidth(0, 200);
    logTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    logTable->setColumnWidth(1, 100);
    logTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    logTable->setColumnWidth(2, 100);
    logTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    logTable->setShowGrid(false);

    leftLayout->addWidget(logTable);
    contentLayout->addLayout(leftLayout, 2);

    // 우측 이미지 프리뷰
    imagePreviewLabel = new QLabel("🖼️ 로그를 선택하세요");
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setMinimumWidth(400);  // 너비 고정
    imagePreviewLabel->setStyleSheet("background-color: #1e1e1e; border: 1px solid #555;");
    contentLayout->addWidget(imagePreviewLabel, 1);

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

    // 레이아웃 추가
    outerLayout->addLayout(topLayout);       // 상단 제목/버튼
    outerLayout->addLayout(contentLayout);   // 본문 영역
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
        logTable->setItem(row, 4, new QTableWidgetItem(entry.imageUrl));
        ++row;
    }
}
