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
                    QVBoxLayout *popupLayout = new QVBoxLayout(popup);

                    QLabel *imgLabel = new QLabel();
                    imgLabel->setAlignment(Qt::AlignCenter);
                    popupLayout->addWidget(imgLabel);

                    // ✅ 원본 이미지 저장
                    QPixmap originalPix = pix;
                    imgLabel->setPixmap(originalPix.scaled(800, 600,
                                                           Qt::KeepAspectRatio,
                                                           Qt::SmoothTransformation));

                    // ✅ 슬라이더 추가
                    QLabel *sliderLabel = new QLabel("강도: 50%");
                    sliderLabel->setStyleSheet("color: white;");
                    sliderLabel->setAlignment(Qt::AlignCenter);

                    QSlider *enhanceSlider = new QSlider(Qt::Horizontal);
                    enhanceSlider->setRange(0, 100);
                    enhanceSlider->setValue(50);
                    enhanceSlider->setStyleSheet("QSlider { background: #333; }");

                    popupLayout->addWidget(sliderLabel);
                    popupLayout->addWidget(enhanceSlider);

                    // 슬라이더 값 변경 시 항상 원본 기준으로 보정
                    connect(enhanceSlider, &QSlider::valueChanged, popup, [=](int val) {
                        sliderLabel->setText(QString("강도: %1%").arg(val));
                        if (!originalPix.isNull()) {
                            QPixmap enhanced = ImageEnhancer::enhanceSharpness(originalPix, val);
                            imgLabel->setPixmap(enhanced.scaled(800, 600,
                                                                Qt::KeepAspectRatio,
                                                                Qt::SmoothTransformation));
                        }
                    });

                    popup->resize(820, 700);
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
