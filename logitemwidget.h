#ifndef LOGITEMWIDGET_H
#define LOGITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class LogItemWidget : public QWidget {
    Q_OBJECT
public:
    explicit LogItemWidget(const QString &camera,
                           const QString &event,
                           const QString &time,
                           const QString &imageUrl = "",
                           QWidget *parent = nullptr);};

#endif // LOGITEMWIDGET_H
