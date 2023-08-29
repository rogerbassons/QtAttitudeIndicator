#ifndef XPLANEPROVIDER_H
#define XPLANEPROVIDER_H

#include <QObject>

class AttitudeProvider : public QObject
{
    Q_OBJECT
public:
    explicit AttitudeProvider(QObject *parent = nullptr);
    bool isStopping();
    void unsubscribePosition();

public slots:
    void subscribePosition();

signals:
    void setPosition(qreal pitch, qreal bank);

private:
    bool stopping;
};

#endif // XPLANEPROVIDER_H
