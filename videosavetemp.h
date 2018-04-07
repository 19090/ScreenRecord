#ifndef VIDEOSAVETEMP_H
#define VIDEOSAVETEMP_H

#include <QObject>

class VideoSaveTemp : public QObject
{
    Q_OBJECT
public:
    explicit VideoSaveTemp(QObject *parent = nullptr);

signals:

public slots:

private:
    QString fileName;

};

#endif // VIDEOSAVETEMP_H
