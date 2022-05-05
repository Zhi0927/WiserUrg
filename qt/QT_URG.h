#ifndef ZHI_QT_URGRUN_H_
#define ZHI_QT_URGRUN_H_

#include "ObjectDetector.hpp"
#include "MouseSimulator.hpp"
#include <QtWidgets/QWidget>
#include "ui_QT_URG.h"

class QT_URG : public QWidget
{
    Q_OBJECT
public:
    QT_URG(QWidget *parent = Q_NULLPTR);
    ~QT_URG();

private:
    void drawRect(const Rect& rect, QColor color = Qt::red);
    void drawLabel(const float x, const float y, const QString& text, const int fontsize, const QColor& color);
    void setPlottemplate();
    void setData();
    void clearData(bool item = false);

    void Mainloop();

private slots:
    void ConnectTcp_Button();
    void DisconnectTcp_Button();
    void setConstraintRegion_Button();
    void setParm_Buttom();
    void useOffset(bool checkstate);

    void DrawMain();

public:
    QColor distanceColor = Qt::blue;
    QColor objectColor = Qt::green;
    QColor processedObjectColor = Qt::cyan;
    QColor objectPointColor = Qt::red;

protected:
    virtual void keyPressEvent(QKeyEvent* ev);
    virtual void keyReleaseEvent(QKeyEvent* ev);

private:
    Ui::Qt_urgClass* ui;
    std::unique_ptr <URGSensorObjectDetector>   UrgDetector;
    std::unique_ptr<UrgDeviceEthernet>		    UrgNet;

    std::vector<long>						    Origindistance;
    int                                         scanstep = 0;

    bool isdetect = false;

    QVector<double> PointX, PointY;
    QVector<double> RawObjX, RawObjY;
    QVector<double> PosObjX, PosObjY;
    QVector<double> ObjPointX, ObjPointY;

    QPointer<QCPItemRect> xRectItem = nullptr;
    QPointer<QCPItemText> xLabelItem = nullptr;
};
#endif
