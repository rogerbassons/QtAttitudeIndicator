#include "QAttitudeIndicator.h"
#include "./ui_QAttitudeIndicator.h"
#include<math.h>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainterPath>
#include <QGraphicsTextItem>
#include <QThread>

QAttitudeIndicator::QAttitudeIndicator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QAttitudeIndicator)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);

    updatePosition(0,0);

    QThread* thread = new QThread();
    provider = new AttitudeProvider();
    provider->moveToThread(thread);
    connect(thread, &QThread::started, provider, &AttitudeProvider::subscribePosition);
    connect(provider, &AttitudeProvider::setPosition, this, &QAttitudeIndicator::updatePosition);
    thread->start();
}

qreal calcPosition(qreal initialPos, qreal maxDistance, qreal bankAngle, qreal pitchAngle, bool cosine) {
    qreal distance = maxDistance / 2;

    float cosAngle = cos(bankAngle + pitchAngle);
    float sinAngle = sin(bankAngle + pitchAngle);
    if ( distance * fabs(sinAngle) <= distance * fabs(cosAngle))
    {
        distance = distance / fabs(cosAngle);
    }
    else
    {
        distance = distance / fabs(sinAngle);
    }

    if (cosine) {
        distance = distance * cosAngle;
    } else {
        distance = distance * sinAngle;
    }

    qreal pos = initialPos + distance;
    if (fabs(pos) < 0.01) {
        pos = 0;
    } else if (maxDistance - fabs(pos) < 0.01) {
        pos = maxDistance;
    }

    return pos;
}

void QAttitudeIndicator::updatePosition(qreal pitch, qreal bank) {
    int width = this->size().width();
    int height = this->size().height();

    scene->clear();

    QRectF sceneRect = QRectF(0,0, width, height);

    scene->setSceneRect(sceneRect);
    scene->setBackgroundBrush(Qt::black);

    qreal x = width / 2.0;
    qreal y = height / 2.0;
    qreal displayedDegrees = 45;
    qreal distancePerDegree = (height / 2) / displayedDegrees;

    QPointF centerScale;
    QPointF leftScale;
    QPointF rightScale;

    qreal pitchAngle = acos(pitch * M_PI / 180);
    qreal bankAngle = (90 - bank) * M_PI / 180; 

    qreal x1 = calcPosition(x, width, bankAngle, pitchAngle, true);
    qreal y1 = calcPosition(y, height, bankAngle, pitchAngle, false);

    qreal x2 = calcPosition(x, width, bankAngle, -pitchAngle, true);
    qreal y2 = calcPosition(y, height, bankAngle, -pitchAngle, false);

    QPointF q1 = QPointF(x1, y1);
    QPointF q2 = QPointF(x2, y2);

    qreal scaleHeight = 1;
    QPainterPath *scaleZero = new QPainterPath();
    scaleZero->moveTo(q1);
    scaleZero->lineTo(q2);

    if (q1.rx() > q2.rx()) {
        leftScale = q2;
        rightScale = q1;
    } else if (q1.rx() <= q2.rx()) {
        leftScale = q1;
        rightScale = q2;
    }

    QPainterPath *sky = new QPainterPath();
    QPainterPath *ground = new QPainterPath();

    sky->moveTo(rightScale);
    sky->lineTo(leftScale);
    ground->moveTo(rightScale);
    ground->lineTo(leftScale);

    if (leftScale.ry() == 0 && leftScale.rx() != 0) {
        sky->lineTo(width, 0);
        ground->lineTo(0, 0);
        ground->lineTo(0, height);
    } else if (leftScale.ry() == height && leftScale.rx() != 0) {
        sky->lineTo(0, height);
        sky->lineTo(0, 0);
        ground->lineTo(width, height);
    } else {
        sky->lineTo(0,0);
        ground->lineTo(0, height);
    }

    if (rightScale.ry() == 0 && rightScale.rx() != width) {
        ground->lineTo(width, height);
        ground->lineTo(width, 0);
    } else if (rightScale.ry() == height && rightScale.rx() != width) {
        sky->lineTo(width, 0);
        sky->lineTo(width, height);
    } else {
        sky->lineTo(width, 0);
        ground->lineTo(0, height);
        ground->lineTo(width, height);
    }

    if (bank > 90 || bank < -90) {
        QPainterPath *tmp = ground;
        ground = sky;
        sky = tmp;
    }


    QBrush skyBrush;
    QPen skyPen;
    QColor skyBrushColor;
    skyBrushColor.setRgb(42,101,177);
    skyBrush.setColor(skyBrushColor);
    skyBrush.setStyle(Qt::SolidPattern);
    skyPen.setColor(skyBrushColor);

    QBrush groundBrush;
    QPen groundPen;
    QColor groundBrushColor;
    groundBrushColor.setRgb (95,49,15);
    groundBrush.setColor(groundBrushColor);
    groundBrush.setStyle(Qt::SolidPattern);
    groundPen.setColor(groundBrushColor);

    scene->addPath(*sky, skyPen, skyBrush);
    scene->addPath(*ground, groundPen, groundBrushColor);
    delete sky;
    delete ground;

    scene->addPath(*scaleZero, QPen(Qt::white), QBrush(Qt::white));
    delete scaleZero;
    scene->update();
}

QAttitudeIndicator::~QAttitudeIndicator()
{
    delete ui;
    provider->unsubscribePosition();
}

