#include "logitemwidget.h"
#include "clickablelabel.h"
#include "imageenhancer.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QMouseEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>

LogItemWidget::LogItemWidget(const QString &camera,
                             const QString &event,
                             const QString &time,
                             const QString &imageUrl,
                             QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel *cameraLabel = new QLabel("📷 " + camera);
    cameraLabel->setStyleSheet(R"(
        font-weight: bold;
        color: white;
        font-size: 12px;
        border-top: 1px solid #333;
        padding-top: 4px;
    )");

    QLabel *eventLabel = new QLabel(event);
    eventLabel->setStyleSheet("color: orange; font-size: 12px;");

    QLabel *timeLabel = new QLabel(time);
    timeLabel->setStyleSheet(R"(
        color: gray;
        font-size: 11px;
        border-bottom: 1px solid #333;
        padding-bottom: 4px;
    )");

    layout->addWidget(cameraLabel);
    layout->addWidget(eventLabel);
    layout->addWidget(timeLabel);

    if (!imageUrl.isEmpty()) {
        ClickableLabel *thumbLabel = new ClickableLabel();
        thumbLabel->setFixedSize(160, 120);
        thumbLabel->setStyleSheet("background-color: #333; border: 1px solid #555;");
        thumbLabel->setCursor(Qt::PointingHandCursor);
        layout->addWidget(thumbLabel, 0, Qt::AlignHCenter);

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkRequest request((QUrl(imageUrl)));
        QNetworkReply *reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            QPixmap pix;
            pix.loadFromData(reply->readAll());
            if (!pix.isNull()) {
                thumbLabel->setPixmap(pix.scaled(160, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));

                // ✅ 클릭 시 팝업 띄우기
                connect(thumbLabel, &ClickableLabel::clicked, this, [=]() {
                    QDialog *popup = new QDialog(this);
                    popup->setWindowTitle("이미지 미리보기");
                    popup->setStyleSheet("background-color: black;");
                    popup->resize(320, 240);

                    QVBoxLayout *popupLayout = new QVBoxLayout(popup);

                    QLabel *imgLabel = new QLabel();
                    imgLabel->setAlignment(Qt::AlignCenter);
                    popupLayout->addWidget(imgLabel);

                    // ✅ 원본 이미지 저장
                    QPixmap originalPix = pix;
                    imgLabel->setPixmap(originalPix.scaled(320, 240,
                                                           Qt::KeepAspectRatio,
                                                           Qt::SmoothTransformation));

                    // 🔹 샤프닝 슬라이더
                    QLabel *sharpLabel = new QLabel("샤프닝: 0");
                    sharpLabel->setStyleSheet("color: #f37321; font-size: 11px;");
                    sharpLabel->setAlignment(Qt::AlignCenter);
                    popupLayout->addWidget(sharpLabel);

                    QSlider *sharpSlider = new QSlider(Qt::Horizontal);
                    sharpSlider->setRange(-100, 100);
                    sharpSlider->setValue(0);
                    sharpSlider->setStyleSheet("QSlider { background: #1e1e1e; }");
                    popupLayout->addWidget(sharpSlider);

                    // 🔹 대비 슬라이더
                    QLabel *contrastLabel = new QLabel("대비: 0");
                    contrastLabel->setStyleSheet("color: #f37321; font-size: 11px;");
                    contrastLabel->setAlignment(Qt::AlignCenter);
                    popupLayout->addWidget(contrastLabel);

                    QSlider *contrastSlider = new QSlider(Qt::Horizontal);
                    contrastSlider->setRange(-100, 100);
                    contrastSlider->setValue(0);
                    contrastSlider->setStyleSheet("QSlider { background: #1e1e1e; }");
                    popupLayout->addWidget(contrastSlider);

                    // ✅ 슬라이더 값 변경 시 동시 적용
                    auto applyEnhancements = [=]() {
                        if (!originalPix.isNull()) {
                            int sharpVal = sharpSlider->value();
                            int contrastVal = contrastSlider->value();

                            QPixmap processed = ImageEnhancer::enhanceSharpness(originalPix, sharpVal);
                            processed = ImageEnhancer::enhanceCLAHE(processed, contrastVal);

                            sharpLabel->setText(QString("샤프닝: %1").arg(sharpVal));
                            contrastLabel->setText(QString("대비: %1").arg(contrastVal));

                            imgLabel->setPixmap(processed.scaled(320, 240,
                                                                 Qt::KeepAspectRatio,
                                                                 Qt::SmoothTransformation));
                        }
                    };

                    connect(sharpSlider, &QSlider::valueChanged, popup, applyEnhancements);
                    connect(contrastSlider, &QSlider::valueChanged, popup, applyEnhancements);

                    popup->exec();
                });

            } else {
                thumbLabel->setText("❌ 이미지 없음");
                thumbLabel->setAlignment(Qt::AlignCenter);
            }
        });
    }

    setFixedHeight(80 + (imageUrl.isEmpty() ? 0 : 130));
    setStyleSheet("background-color: #1e1e1e;");
}
