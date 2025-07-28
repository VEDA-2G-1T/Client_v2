#ifndef CAMERAREGISTRATIONDIALOG_H
#define CAMERAREGISTRATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QMouseEvent>

class CameraRegistrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraRegistrationDialog(QWidget *parent = nullptr);

    QString getCameraName() const;
    QString getCameraIP() const;
    QString getCameraPort() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();

    QLineEdit *nameEdit;
    QLineEdit *ipEdit;
    QLineEdit *portEdit;

    QPushButton *okButton;
    QPushButton *cancelButton;

    bool dragging = false;
    QPoint dragPosition;
};

#endif // CAMERAREGISTRATIONDIALOG_H
