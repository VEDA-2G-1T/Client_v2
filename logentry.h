#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QString>

struct LogEntry {
    QString cameraName;
    QString function;
    QString event;
    QString timestamp;
    QString imageUrl;
};

#endif // LOGENTRY_H
