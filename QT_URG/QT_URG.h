#ifndef ZHI_QT_URGRUN_H_
#define ZHI_QT_URGRUN_H_

#include "EthernetConnector.hpp"
#include "SCIP_library.hpp"
#include "ObjectDetector.hpp"
#include "WinHIDController.hpp"
#include "ConfigManager.hpp"

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
    void drawLabel(QPointer<QCPItemText>& item, const float x, const float y, const QString& text, int fontsize = 16,  QColor color = Qt::black);
    void InitPlot();
    void setAllPlotData();
    void clearAllPlotData();

    void UrgMain();

protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void ConnectTcp01_Button();
    void DisconnectTcp01_Button();
    void ConnectTcp02_Button();
    void DisconnectTcp02_Button();

    void setConstraintRegion_Button();
    void setParm_Buttom();
    void useOffset(bool checkstate);
    void useTouchEvent(bool checkstate);

    void Update();

public:
    QColor distanceColor01              = Qt::green;
    QColor distanceColor02              = Qt::yellow;
    QColor disboarderColor              = Qt::magenta;
    QColor objectColor                  = Qt::gray;
    QColor objectboarderColor           = Qt::darkGray;
    QColor objectPointColor             = Qt::blue;
    QColor processedObjectColor         = Qt::cyan;  
    QColor processedObjectboarderColor  = Qt::darkCyan;
    QColor RectColor                    = Qt::red;
    QColor LabelColor                   = Qt::black;

private:
    Ui::Qt_urgClass* ui;

    std::unique_ptr<ObjectDetector>     UrgDetector;
    std::unique_ptr<MouseSimulator>		UrgMouse;
    std::unique_ptr<HotkeyManager>		UrgKeyBoard;
    std::unique_ptr<EthernetConnector>	UrgNet01;
    std::unique_ptr<EthernetConnector>	UrgNet02;
    std::mutex                          distance_guard;

    std::vector<long>					Origindistance01;
    std::vector<long>					Origindistance02;
    std::vector<long>					Previewdistance01;
    std::vector<long>					Previewdistance02;
    std::vector<vector3>                Directions;

    bool                                RegionInverse           = false;  //part1 left, part2 right

    QVector<double>                     PointX01,  PointY01;
    QVector<double>                     PointX02,  PointY02;
    QVector<double>                     RawObjX,   RawObjY;
    QVector<double>                     PosObjX,   PosObjY;
    QVector<double>                     ObjPointX, ObjPointY;

    QPointer<QCPItemRect>               RectItem                = nullptr;
    QPointer<QCPItemText>               FPSItem                 = nullptr;
    QPointer<QCPItemText>               LabelItem01             = nullptr;
    QPointer<QCPItemText>               LabelItem02             = nullptr;
    QPointer<QCPCurve>                  Curveitem01             = nullptr;
    QPointer<QCPCurve>                  Curveitem02             = nullptr;
};
#endif
