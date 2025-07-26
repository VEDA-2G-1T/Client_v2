#include "loghistorydialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QPixmap>
#include <QNetworkReply>
#include <QUrl>
#include <QFontDatabase>

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
            color: #f37321;
        }
        QHeaderView::section {
            background-color: #2b2b2b;
            color: white;
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
    int idB = QFontDatabase::addApplicationFont(":/resources/fonts/01HanwhaB.ttf");
    int gidR = QFontDatabase::addApplicationFont(":/resources/fonts/05HanwhaGothicR.ttf");
    int gidL = QFontDatabase::addApplicationFont(":/resources/fonts/06HanwhaGothicL.ttf");

    QString fontB = QFontDatabase::applicationFontFamilies(idB).at(0);
    QString gfontR = QFontDatabase::applicationFontFamilies(gidR).at(0);
    QString gfontL = QFontDatabase::applicationFontFamilies(gidL).at(0);

    QFont titleFont(fontB, 15);
    QFont tableFont(gfontR, 10);
    QFont buttonFont(gfontL, 8);
    QFont previewFont(gfontR, 15);

    // 🔹 전체를 감싸는 수직 레이아웃
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(5, 5, 0, 0);
    outerLayout->setSpacing(5);

    // 🔹 상단: 제목 + 닫기 버튼 (수평)
    QHBoxLayout *topLayout = new QHBoxLayout();
    QLabel *title = new QLabel("Event Log History");
    title->setFont(titleFont);  // ✅ 타이틀

    QPushButton *closeButton = new QPushButton("닫기");
    closeButton->setFont(buttonFont);  // ✅ 버튼

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
    logTable->setFont(tableFont);  // ✅ 테이블 전체

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
    imagePreviewLabel = new QLabel("Select Event Log");
    imagePreviewLabel->setFont(previewFont);  // ✅ 이미지 안내

    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setMinimumWidth(320);  // 너비 고정
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
    int gidR = QFontDatabase::addApplicationFont(":/resources/fonts/05HanwhaGothicR.ttf");
    QString gfontR = QFontDatabase::applicationFontFamilies(gidR).at(0);
    QFont tableContentsFont(gfontR, 8);

    logTable->setRowCount(logs.size());
    int row = 0;
    for (int row = 0; row < logs.size(); ++row) {
        const LogEntry &entry = logs[row];

        QTableWidgetItem *item0 = new QTableWidgetItem(entry.timestamp);
        item0->setFont(tableContentsFont);
        logTable->setItem(row, 0, item0);

        QTableWidgetItem *item1 = new QTableWidgetItem(entry.cameraName);
        item1->setFont(tableContentsFont);
        logTable->setItem(row, 1, item1);

        QTableWidgetItem *item2 = new QTableWidgetItem(entry.function);
        item2->setFont(tableContentsFont);
        logTable->setItem(row, 2, item2);

        QTableWidgetItem *item3 = new QTableWidgetItem(entry.event);
        item3->setFont(tableContentsFont);
        logTable->setItem(row, 3, item3);

        QTableWidgetItem *item4 = new QTableWidgetItem(entry.imageUrl);
        item4->setFont(tableContentsFont);
        logTable->setItem(row, 4, item4);
    }

}
