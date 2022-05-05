#include "QT_URG.h"
#include <QMessageBox>

QT_URG::QT_URG(QWidget *parent)
    :   QWidget(parent),
        ui(new Ui::Qt_urgClass)
{
    //======================================= * Awake * ===========================================//
    ui->setupUi(this);
    this->setWindowIcon(QIcon("icon_urg.jpg"));
    UrgDetector.reset(new URGSensorObjectDetector());

    //======================================= * Start * ===========================================//
    isdetect = false;
    setPlottemplate();

    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->setInteraction(QCP::iRangeZoom, true);
    ui->plot->setPlottingHint(QCP::phFastPolylines);
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");

    connect(ui->Connect_Button, SIGNAL(clicked()), this, SLOT(ConnectTcp_Button()));
    connect(ui->Disconnect_Button, SIGNAL(clicked()), this, SLOT(DisconnectTcp_Button()));
    connect(ui->SetCR_Button, SIGNAL(clicked()), this, SLOT(setConstraintRegion_Button()));
    connect(ui->SetParm_Button, SIGNAL(clicked()), this, SLOT(setParm_Buttom()));
    connect(ui->UseOffset, SIGNAL(clicked(bool)), this, SLOT(useOffset(bool)));

    ui->Connect_Button->setEnabled(true);
    ui->Disconnect_Button->setEnabled(false);
    ui->SetCR_Button->setEnabled(false);
    ui->SetParm_Button->setEnabled(false);

    //======================================= * loop * ===========================================//
    QTimer* dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(DrawMain()));
    dataTimer->start(0);

    std::cout << "Initial QT successful!" << std::endl;
}

void QT_URG::DrawMain() {
    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0;
    static double lastFpsKey = 0;
    static int frameCount;
    ++frameCount;

    if (UrgNet != nullptr) {
        if (UrgNet->GetConnectState()) {
            Mainloop();
            const std::vector<vector3>& directions = UrgDetector->GetDirection();
            const std::vector<RawObject>& rawObjectList = UrgDetector->GetRawObjectList();
            const std::vector<ProcessedObject>& detectedObjects = UrgDetector->GetDetectObjects();

            if (ui->DrawPoint->isChecked() && !Origindistance.empty()) {
                for (int i = 0; i < Origindistance.size(); i++) {
                    vector3 result = directions[i] * Origindistance[i];
                    PointX.append(static_cast<double>(result.x));
                    PointY.append(static_cast<double>(result.y));
                }
            }

            if (ui->DrawObject->isChecked() && !rawObjectList.empty()) {
                for (int i = 0; i < rawObjectList.size(); i++) {
                    auto obj = rawObjectList[i];
                    if (obj.dirList.size() == 0 || obj.distList.size() == 0) return;
                    auto rawpos = obj.getPosition();
                    RawObjX.append(static_cast<double>(rawpos.x));
                    RawObjY.append(static_cast<double>(rawpos.y));

                    for (size_t i = 0; i < obj.distList.size(); i++) {
                        auto detectpos = obj.dirList[i] * obj.distList[i];
                        ObjPointX.append(static_cast<double>(detectpos.x));
                        ObjPointY.append(static_cast<double>(detectpos.y));
                    }
                }
            }

            if (ui->DrawProObject->isChecked()) {
                for (auto pObj : detectedObjects) {
                    auto propos = pObj.getPosition();
                    PosObjX.append(static_cast<double>(propos.x));
                    PosObjY.append(static_cast<double>(propos.y));
                }
            }
            setData();
            ui->plot->replot();
            clearData();
        }
        else {
            DisconnectTcp_Button();
        }
    }

    ui->FPS_Label->setText(QString("FPS: %1").arg(frameCount / (key - lastFpsKey), 0, 'f', 0));
    lastFpsKey = key;
    frameCount = 0;
}
//============================================================================================//

QT_URG::~QT_URG() {
    delete ui;
    UrgDetector.reset(nullptr);
    UrgNet.reset(nullptr);
}



void QT_URG::ConnectTcp_Button() {
    if (UrgNet == nullptr) {
        UrgNet.reset(new UrgDeviceEthernet(ui->IP_Input->text().toStdString(), ui->Port_number_input->value()));

        if (UrgNet->StartTCP()) {
            UrgNet->StartMeasureDistance();

            drawLabel(0, 0, "Radar", 16, Qt::black);
            setConstraintRegion_Button();
            setParm_Buttom();
            ui->Connect_Button->setEnabled(false);   
            ui->Disconnect_Button->setEnabled(true);
            ui->SetCR_Button->setEnabled(true);
            ui->SetParm_Button->setEnabled(true);
        }
        else {
            UrgNet.reset(nullptr);
            QMessageBox::information(this, tr("IMD"), tr("Connection failed.\nReason: Connection to socked failed."));
        }
    }
}

void QT_URG::DisconnectTcp_Button() {
    if (UrgNet != nullptr) {
        UrgNet.reset(nullptr);
        ui->Connect_Button->setEnabled(true);
        ui->Disconnect_Button->setEnabled(false);
        ui->SetCR_Button->setEnabled(false);
        ui->SetParm_Button->setEnabled(false);

        clearData(true);
        setData();
        ui->plot->replot();
    }
}

void QT_URG::setConstraintRegion_Button() {
    UrgDetector->parm.detctRect.xmin = -(ui->Width_Input->value() / 2) + ui->OffsetX_Input->value();
    UrgDetector->parm.detctRect.ymin = ui->Height_Input->value() + ui->OffsetY_Input->value();
    UrgDetector->parm.detctRect.width = ui->Width_Input->value();
    UrgDetector->parm.detctRect.height = ui->Height_Input->value();

    Rect rect = UrgDetector->parm.detctRect;

    if (xRectItem != nullptr) {
        xRectItem->topLeft->setCoords(rect.xmin, rect.ymin);
        xRectItem->bottomRight->setCoords(rect.xmax(), rect.ymax());
    }
    else {
        drawRect(rect, Qt::red);
    }
}

void QT_URG::setParm_Buttom() {
    UrgDetector->parm.noiseLimit        = ui->noiseLimit_Input->value();
    UrgDetector->parm.deltaLimit        = ui->deltaLimit_Input->value();
    UrgDetector->parm.distanceThreshold = ui->distanceThreshold_Input->value();
    UrgDetector->parm.detectsize        = ui->DetectSize_Input->value();

    UrgDetector->parm.screenWidth       = ui->ResolutionWidth_Input->value();
    UrgDetector->parm.screenHeight      = ui->ResolutionHeight_Input->value();
}


void QT_URG::setPlottemplate() {
    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssSquare, QPen(objectColor, 2), objectColor, 15));
    ui->plot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsNone);

    ui->plot->addGraph();
    ui->plot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssSquare, QPen(processedObjectColor, 2), processedObjectColor, 15));
    ui->plot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsNone);

    ui->plot->addGraph();
    ui->plot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssPlus, QPen(distanceColor, 1), distanceColor, 2));
    ui->plot->graph(2)->setLineStyle(QCPGraph::LineStyle::lsNone);

    ui->plot->addGraph();
    ui->plot->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssPlus, QPen(objectPointColor, 1), objectPointColor, 4));
    ui->plot->graph(3)->setLineStyle(QCPGraph::LineStyle::lsNone);

    PointX.reserve(1081);
    PointX.reserve(1081);
}

void QT_URG::setData() {
    ui->plot->graph(0)->setData(RawObjX, RawObjY, true);
    ui->plot->graph(1)->setData(PosObjX, PosObjY, true);
    ui->plot->graph(2)->setData(PointX, PointY, true);
    ui->plot->graph(3)->setData(ObjPointX, ObjPointY, true);

    ui->plot->replot();
}

void QT_URG::clearData(bool item) {
    RawObjX.clear();
    RawObjY.clear();
    PosObjX.clear();
    PosObjY.clear();
    PointX.clear();
    PointY.clear();
    ObjPointX.clear();
    ObjPointY.clear();

    if (item) ui->plot->clearItems();
}

void QT_URG::drawRect(const Rect& rect, QColor color) {
    xRectItem = new QCPItemRect(ui->plot);
    xRectItem->setVisible(true);
    xRectItem->setPen(QPen(color));
    xRectItem->setBrush(QBrush(Qt::NoBrush));

    xRectItem->topLeft->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->topLeft->setCoords(rect.xmin, rect.ymin);

    xRectItem->bottomRight->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->bottomRight->setCoords(rect.xmax(), rect.ymax());
}

void QT_URG::drawLabel(const float x, const float y, const QString& text, const int fontsize, const QColor& color) {
    xLabelItem = new QCPItemText(ui->plot);
    xLabelItem->setPositionAlignment(Qt::AlignHCenter | Qt::AlignHCenter);
    xLabelItem->position->setType(QCPItemPosition::ptPlotCoords);
    xLabelItem->position->setCoords(x, y);
    xLabelItem->setText(text);
    xLabelItem->setFont(QFont(font().family(), fontsize));
    xLabelItem->setPen(QPen(color));
}


void QT_URG::useOffset(bool checkstate) {
    UrgDetector->parm.useOffset = checkstate;
    if (checkstate) {
        UrgDetector->parm.positionOffset.x = ui->cursoroffsetX_Input->value();
        UrgDetector->parm.positionOffset.y = ui->cursoroffsetY_Input->value();
    }
}

void QT_URG::keyPressEvent(QKeyEvent* ev)
{
    if (ev->key() == Qt::Key_Q)
    {
        
        return;
    }

    QWidget::keyPressEvent(ev);
}

void QT_URG::keyReleaseEvent(QKeyEvent* ev)
{
    //if (ev->key() == Qt::Key_W)
    //{
    //    
    //    return;
    //}

    //QWidget::keyReleaseEvent(ev);
}

void QT_URG::Mainloop() {

    std::unique_lock<std::mutex> lockurg(UrgNet->distance_guard);

    if (UrgNet->recv_distances.size() <= 0) return;
    Origindistance = UrgNet->recv_distances;

    lockurg.unlock();

    if (Origindistance.size() <= 0) return;

    if (scanstep <= 0) {
        scanstep = UrgNet->recv_distances.size();
        UrgDetector->CacheDirections(scanstep);
    }

    UrgDetector->UpdateObjectList(Origindistance);
}