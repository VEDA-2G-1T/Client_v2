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
#include <QTabWidget>
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QTimer>
#include <QMouseEvent>

LogHistoryDialog::LogHistoryDialog(const QVector<LogEntry> &logs, QWidget *parent)
    : QDialog(parent), allLogs(logs)
{
    // ✅ Frameless 적용
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setFixedSize(1200, 700);
    setStyleSheet(R"(
        QDialog {
            background-color: #1e1e1e;
            color: white;
            border: 2px solid #444;
            border-radius: 6px;
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
        QTabWidget::pane {
            border: none;
            background-color: #2b2b2b;
        }
        QTabBar::tab {
            background-color: #2b2b2b;
            color: white;
            padding: 6px 12px;
            border: 1px solid #444;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #1e1e1e;
            color: #f37321;
            font-weight: bold;
        }
        QTabBar::tab:hover {
            background-color: #3a3a3a;
        }
    )");

    setupUI();
    populateTabs();
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
    QFont buttonFont(gfontL, 8);
    QFont previewFont(gfontR, 15);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(5, 5, 0, 10);
    outerLayout->setSpacing(5);

    // ✅ 상단바: 타이틀 + 닫기 버튼
    QWidget *topBar = new QWidget();
    topBar->setFixedHeight(28);
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(5, 0, 5, 0);

    QLabel *title = new QLabel("Event Log History");
    title->setFont(titleFont);

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(24, 24);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: white;
            border: none;
        }
        QPushButton:hover {
            color: #f37321;
        }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    topLayout->addWidget(title);
    topLayout->addStretch();
    topLayout->addWidget(closeBtn);
    outerLayout->addWidget(topBar);

    // 🔹 필터 체크박스
    QVBoxLayout *filterLayout = new QVBoxLayout();

    totalCheck = new QCheckBox("Total");
    blurCheck = new QCheckBox("Blur");
    ppeCheck = new QCheckBox("Detect");
    trespassCheck = new QCheckBox("Trespass");
    fallCheck = new QCheckBox("Fall");

    QString checkboxStyle = R"(
        QCheckBox {
            color: white;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 14px;
            height: 14px;
        }
    )";

    totalCheck->setStyleSheet(checkboxStyle);
    blurCheck->setStyleSheet(checkboxStyle);
    ppeCheck->setStyleSheet(checkboxStyle);
    trespassCheck->setStyleSheet(checkboxStyle);
    fallCheck->setStyleSheet(checkboxStyle);

    totalCheck->setChecked(true);

    filterLayout->addWidget(totalCheck);
    filterLayout->addWidget(blurCheck);
    filterLayout->addWidget(ppeCheck);
    filterLayout->addWidget(trespassCheck);
    filterLayout->addWidget(fallCheck);

    QWidget *filterWidget = new QWidget();
    filterWidget->setLayout(filterLayout);
    filterWidget->setFixedWidth(100);
    filterWidget->setStyleSheet("background-color: transparent;");
    filterWidget->setFixedHeight(460);

    // ✅ applyFilter 호출을 지연 처리 (체크 상태가 최종 반영된 후 실행)
    auto delayedApplyFilter = [=]() {
        QTimer::singleShot(0, this, [=]() {
            applyFilter();
        });
    };

    connect(totalCheck,     &QCheckBox::checkStateChanged, this, delayedApplyFilter);
    connect(blurCheck,      &QCheckBox::checkStateChanged, this, delayedApplyFilter);
    connect(ppeCheck,       &QCheckBox::checkStateChanged, this, delayedApplyFilter);
    connect(trespassCheck,  &QCheckBox::checkStateChanged, this, delayedApplyFilter);
    connect(fallCheck,      &QCheckBox::checkStateChanged, this, delayedApplyFilter);

    tabWidget = new QTabWidget(this);
    connect(tabWidget, &QTabWidget::currentChanged, this, &LogHistoryDialog::applyFilter);

    imagePreviewLabel = new QLabel("Select Event Log");
    imagePreviewLabel->setFont(previewFont);
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setMinimumWidth(320);
    imagePreviewLabel->setStyleSheet("background-color: #1e1e1e; border: 1px solid #555;");


    previewManager = new QNetworkAccessManager(this);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->addWidget(filterWidget, 0);
    contentLayout->addWidget(tabWidget, 3);
    contentLayout->addWidget(imagePreviewLabel, 1);

    outerLayout->addLayout(contentLayout);

    // ✅ Total 상태 동기화 로직 유지
    connect(totalCheck, &QCheckBox::checkStateChanged, this, [=](int state) {
        if (state == Qt::Checked) {
            blurCheck->setChecked(false);
            ppeCheck->setChecked(false);
            trespassCheck->setChecked(false);
            fallCheck->setChecked(false);
        }
    });

    auto disableTotalIfAnyChecked = [=]() {
        if (blurCheck->isChecked() || ppeCheck->isChecked() ||
            trespassCheck->isChecked() || fallCheck->isChecked()) {
            totalCheck->blockSignals(true);
            totalCheck->setChecked(false);
            totalCheck->blockSignals(false);
        }
    };

    auto checkIfAllChecked = [=]() {
        QTimer::singleShot(0, this, [=]() {
            if (blurCheck->isChecked() && ppeCheck->isChecked() &&
                trespassCheck->isChecked() && fallCheck->isChecked()) {
                disconnect(totalCheck, nullptr, nullptr, nullptr);
                totalCheck->blockSignals(true);
                totalCheck->setChecked(true);
                totalCheck->blockSignals(false);

                blurCheck->blockSignals(true); blurCheck->setChecked(false); blurCheck->blockSignals(false);
                ppeCheck->blockSignals(true);  ppeCheck->setChecked(false);  ppeCheck->blockSignals(false);
                trespassCheck->blockSignals(true); trespassCheck->setChecked(false); trespassCheck->blockSignals(false);
                fallCheck->blockSignals(true); fallCheck->setChecked(false); fallCheck->blockSignals(false);

                connect(totalCheck, &QCheckBox::checkStateChanged, this, delayedApplyFilter);
            }
        });
    };

    connect(blurCheck,      &QCheckBox::checkStateChanged, this, disableTotalIfAnyChecked);
    connect(ppeCheck,       &QCheckBox::checkStateChanged, this, disableTotalIfAnyChecked);
    connect(trespassCheck,  &QCheckBox::checkStateChanged, this, disableTotalIfAnyChecked);
    connect(fallCheck,      &QCheckBox::checkStateChanged, this, disableTotalIfAnyChecked);

    connect(blurCheck,      &QCheckBox::checkStateChanged, this, checkIfAllChecked);
    connect(ppeCheck,       &QCheckBox::checkStateChanged, this, checkIfAllChecked);
    connect(trespassCheck,  &QCheckBox::checkStateChanged, this, checkIfAllChecked);
    connect(fallCheck,      &QCheckBox::checkStateChanged, this, checkIfAllChecked);
}

// ✅ 드래그 이동 지원
void LogHistoryDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void LogHistoryDialog::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}
void LogHistoryDialog::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging = false;
        event->accept();
    }
}

void LogHistoryDialog::populateTabs()
{
    tabWidget->clear();

    QStringList cameraNames = {"전체"};
    for (const LogEntry &entry : allLogs) {
        if (!cameraNames.contains(entry.cameraName))
            cameraNames.append(entry.cameraName);
    }

    for (const QString &name : cameraNames) {
        QTableWidget *table = new QTableWidget();
        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({"Time", "Camera", "Function", "Event", "ImageURL"});
        table->setColumnHidden(4, true);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
        table->setColumnWidth(0, 200);
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
        table->setColumnWidth(1, 100);
        table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
        table->setColumnWidth(2, 100);
        table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setShowGrid(false);

        connect(table, &QTableWidget::cellClicked, this, &LogHistoryDialog::handleRowClick);
        tabWidget->addTab(table, name);
    }

    applyFilter();
}

void LogHistoryDialog::applyFilter()
{
    QString selectedCamera = tabWidget->tabText(tabWidget->currentIndex());

    // 🔸 필터 체크박스 상태
    bool showTotal     = totalCheck->isChecked();
    bool showBlur      = blurCheck->isChecked();
    bool showPPE       = ppeCheck->isChecked();
    bool showTrespass  = trespassCheck->isChecked();
    bool showFall      = fallCheck->isChecked();

    QTableWidget *table = qobject_cast<QTableWidget *>(tabWidget->currentWidget());
    if (!table) return;

    int gidR = QFontDatabase::addApplicationFont(":/resources/fonts/05HanwhaGothicR.ttf");
    QString gfontR = QFontDatabase::applicationFontFamilies(gidR).at(0);
    QFont tableContentsFont(gfontR, 8);

    table->setRowCount(0);
    int row = 0;

    for (const LogEntry &entry : allLogs) {
        if (selectedCamera != "전체" && entry.cameraName != selectedCamera)
            continue;

        // 🔸 Total이 체크되어 있지 않으면 각 항목별 개별 체크
        if (!showTotal) {
            if (entry.function == "Blur" && !showBlur) continue;
            if (entry.function == "PPE" && !showPPE) continue;
            if (entry.function == "Trespass" && !showTrespass) continue;
            if (entry.function == "Fall" && !showFall) continue;
        }

        table->insertRow(row);
        auto *item0 = new QTableWidgetItem(entry.timestamp);
        auto *item1 = new QTableWidgetItem(entry.cameraName);
        auto *item2 = new QTableWidgetItem(entry.function);
        auto *item3 = new QTableWidgetItem(entry.event);
        auto *item4 = new QTableWidgetItem(entry.imageUrl);

        item0->setFont(tableContentsFont);
        item1->setFont(tableContentsFont);
        item2->setFont(tableContentsFont);
        item3->setFont(tableContentsFont);
        item4->setFont(tableContentsFont);

        table->setItem(row, 0, item0);
        table->setItem(row, 1, item1);
        table->setItem(row, 2, item2);
        table->setItem(row, 3, item3);
        table->setItem(row, 4, item4);

        ++row;
    }
}

void LogHistoryDialog::handleRowClick(int row, int)
{
    QTableWidget *table = qobject_cast<QTableWidget *>(sender());
    if (!table) return;
    QString url = table->item(row, 4)->text().trimmed();
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
        if (!pix.isNull())
            imagePreviewLabel->setPixmap(pix.scaled(320, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else
            imagePreviewLabel->setText("❌ 이미지 로드 실패");
        reply->deleteLater();
    });
}
