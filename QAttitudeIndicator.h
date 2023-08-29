#ifndef QAttitudeIndicator_H
#define QAttitudeIndicator_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <XPlaneProvider.h>

QT_BEGIN_NAMESPACE
namespace Ui { class QAttitudeIndicator; }
QT_END_NAMESPACE

class QAttitudeIndicator : public QWidget
{
    Q_OBJECT

public:
    QAttitudeIndicator(QWidget *parent = nullptr);
    ~QAttitudeIndicator();

public slots:
    void updatePosition(qreal pitch, qreal bank);

private:
    Ui::QAttitudeIndicator *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    AttitudeProvider *provider;
};
#endif // QAttitudeIndicator_H
