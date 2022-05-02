#include "QT_URG.h"
#include <QMessageBox>

QT_URG::QT_URG(QWidget *parent)
    :   QWidget(parent),
        ui(new Ui::Qt_urgClass)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon("icon_urg.jpg"));

    //======================================= * start * ===========================================//
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
    connect(ui->UseSMA, SIGNAL(clicked(bool)), this, SLOT(smatoggle(bool)));
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

QT_URG::~QT_URG() {
    delete ui;
}

void QT_URG::DrawMain() {
    if (isdetect) {
        m_urgdetector->mainloop();
        const std::vector<long>& croppedDistances           = m_urgdetector->GetcroppedDistances();
        const std::vector<long>& originDistances            = m_urgdetector->GetOriginDistances();
        const std::vector<vector3>& directions              = m_urgdetector->GetDirection();
        const std::vector<RawObject>& rawObjectList         = m_urgdetector->GetRawObjectList();
        const std::vector<ProcessedObject>& detectedObjects = m_urgdetector->GetDetectObjects();

        //if (ui->DrawPoint->isChecked() && !croppedDistances.empty()) {
        //    for (int i = 0; i < croppedDistances.size(); i++) {
        //        vector3 result = directions[i] * croppedDistances[i];
        //        PointX.append(static_cast<double>(result.x));
        //        PointY.append(static_cast<double>(result.y));
        //    }
        //}
        if (ui->DrawPoint->isChecked() && !originDistances.empty()) {
            for (int i = 0; i < originDistances.size(); i++) {
                vector3 result = directions[i] * originDistances[i];
                PointX.append(static_cast<double>(result.x));
                PointY.append(static_cast<double>(result.y));
            }
        }

        if (ui->DrawObject->isChecked() && !rawObjectList.empty()) {
            for (int i = 0; i < rawObjectList.size(); i++) {
                auto obj = rawObjectList[i];
                if (obj.dirList.size() == 0 || obj.distList.size() == 0)  return;
                auto rawpos = obj.getPosition();
                RawObjX.append(static_cast<double>(rawpos.x));
                RawObjY.append(static_cast<double>(rawpos.y));
            }
        }

        if (ui->DrawProObject->isChecked()) {
            for (auto pObj : detectedObjects) {
                auto propos = pObj.getPosition();
                PosObjX.append(static_cast<double>(propos.x));
                PosObjY.append(static_cast<double>(propos.y));
            }
        }
    }
    plot();
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
    PointX.reserve(1081);
    PointX.reserve(1081);
}

void QT_URG::plot() {
    ui->plot->graph(0)->setData(RawObjX, RawObjY, true);
    ui->plot->graph(1)->setData(PosObjX, PosObjY, true);
    ui->plot->graph(2)->setData(PointX, PointY, true);

    ui->plot->replot();
    RawObjX.clear();
    RawObjY.clear();
    PosObjX.clear();
    PosObjY.clear();
    PointX.clear();
    PointY.clear();
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

void QT_URG::ConnectTcp_Button() {
    if (!isdetect) {
        m_urgdetector.reset(new URGSensorObjectDetector(ui->IP_Input->text().toStdString(), ui->Port_number_input->value()));
        bool sucessful = m_urgdetector->start();

        if (sucessful) {
            setConstraintRegion_Button();
            setParm_Buttom();
            isdetect = true;
            ui->Connect_Button->setEnabled(false);
            ui->Disconnect_Button->setEnabled(true);
            ui->SetCR_Button->setEnabled(true);
            ui->SetParm_Button->setEnabled(true);
        }
        else {
            isdetect = false;
            m_urgdetector.reset(nullptr);
            QMessageBox::information(this, tr("IMD"), tr("Connection failed.\nReason: Connection to socked failed."));
        }
    }
}

void QT_URG::DisconnectTcp_Button() {
    if (isdetect) {
        isdetect = false;
        m_urgdetector.reset(nullptr);

        ui->Connect_Button->setEnabled(true);
        ui->Disconnect_Button->setEnabled(false);
        ui->SetCR_Button->setEnabled(false);
        ui->SetParm_Button->setEnabled(false);
    }
}

void QT_URG::setConstraintRegion_Button() {
    m_urgdetector->parm.detctRect.xmin = -(ui->Width_Input->value() / 2) + ui->OffsetX_Input->value();
    m_urgdetector->parm.detctRect.ymin = ui->Height_Input->value() + ui->OffsetY_Input->value();
    m_urgdetector->parm.detctRect.width = ui->Width_Input->value();
    m_urgdetector->parm.detctRect.height = ui->Height_Input->value();

    Rect rect = m_urgdetector->parm.detctRect;

    if (isdetect) {
        //xRectItem->topLeft->setType(QCPItemPosition::ptPlotCoords);
        xRectItem->topLeft->setCoords(rect.xmin, rect.ymin);
        //xRectItem->bottomRight->setType(QCPItemPosition::ptPlotCoords);
        xRectItem->bottomRight->setCoords(rect.xmax(), rect.ymax());
    }
    else {
        drawRect(rect, Qt::red);
    }
}

void QT_URG::setParm_Buttom() {
    m_urgdetector->parm.noiseLimit = ui->noiseLimit_Input->value();
    m_urgdetector->parm.deltaLimit = ui->deltaLimit_Input->value();
    m_urgdetector->parm.distanceThreshold = ui->distanceThreshold_Input->value();
    m_urgdetector->parm.objPosSmoothTime = ui->objPosSmoothTime_Input->value();
}

void QT_URG::smatoggle(bool checkstate) {
    m_urgdetector->parm.useSMA = checkstate;
}

void QT_URG::useOffset(bool checkstate) {
    m_urgdetector->parm.useOffset = checkstate;
    if (checkstate) {
        m_urgdetector->parm.positionOffset.x = ui->cursoroffsetX_Input->value();
        m_urgdetector->parm.positionOffset.y = ui->cursoroffsetY_Input->value();
    }
}