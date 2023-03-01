#include "QT_URG.h"
#include <QMessageBox>


#pragma region Constructor

QT_URG::QT_URG(QWidget *parent) : QWidget(parent), ui(new Ui::Qt_urgClass){

#pragma region Awake

    ui->setupUi(this);
    this->setWindowIcon(QIcon("icon.png"));
    this->setWindowTitle("WiserURG -version 1.2");

    ConfigManager::Instance()->LoadWindowSize(this);
    ConfigManager::Instance()->LoadParameter(ui);

    UrgDetector.reset(new ObjectDetector());
    UrgMouse.reset(new MouseSimulator());
    UrgKeyBoard.reset(new HotkeyManager());
    KalmanF.reset(new KalmanFilter<long>());

    UrgDetector->CacheDirections();
    Directions = UrgDetector->GetDirection();
    UrgKeyBoard->Init([&](uint32_t process_id, uint32_t key_id, bool ctrl_pressed, bool shift_pressed) {
        if ((char)key_id == 'A' && ctrl_pressed && shift_pressed) {
            bool touchchecked = ui->TouchEvent->isChecked();
            ui->TouchEvent->setChecked(!touchchecked);
            useTouchEvent(!touchchecked);
        }
    });

    InitPlot();
    setConstraintRegion_Spin();
    setParm_Buttom();

#pragma endregion

#pragma region Start

    ui->Connect_Button_01->setEnabled(true);
    ui->Disconnect_Button_01->setEnabled(false);
    ui->Connect_Button_02->setEnabled(true);
    ui->Disconnect_Button_02->setEnabled(false);

    QTimer* dataTimer = new QTimer(this);
    connect(ui->Connect_Button_01,    SIGNAL(clicked()),            this,   SLOT(ConnectTcp01_Button()));
    connect(ui->Disconnect_Button_01, SIGNAL(clicked()),            this,   SLOT(DisconnectTcp01_Button()));
    connect(ui->Connect_Button_02,    SIGNAL(clicked()),            this,   SLOT(ConnectTcp02_Button()));
    connect(ui->Disconnect_Button_02, SIGNAL(clicked()),            this,   SLOT(DisconnectTcp02_Button()));
    connect(ui->OffsetX_Input,        SIGNAL(valueChanged(double)), this,   SLOT(setConstraintRegion_Spin()));
    connect(ui->OffsetY_Input,        SIGNAL(valueChanged(double)), this,   SLOT(setConstraintRegion_Spin()));
    connect(ui->Width_Input,          SIGNAL(valueChanged(double)), this,   SLOT(setConstraintRegion_Spin()));
    connect(ui->Height_Input,         SIGNAL(valueChanged(double)), this,   SLOT(setConstraintRegion_Spin()));
    connect(ui->SetParm_Button,       SIGNAL(clicked()),            this,   SLOT(setParm_Buttom()));
    connect(ui->UseOffset,            SIGNAL(clicked(bool)),        this,   SLOT(useOffset(bool)));
    connect(ui->TouchEvent,           SIGNAL(clicked(bool)),        this,   SLOT(useTouchEvent(bool)));
    connect(ui->UseFlipX,             SIGNAL(clicked(bool)),        this,   SLOT(useFlipX(bool)));
    connect(ui->UseFlipY,             SIGNAL(clicked(bool)),        this,   SLOT(useFlipY(bool)));
    connect(dataTimer,                SIGNAL(timeout()),            this,   SLOT(Update()));
    dataTimer->start(0);

    std::cout << "Initial QT successful!" << std::endl;

#pragma endregion

}

#pragma endregion

#pragma region Loop

void QT_URG::Update() {
#pragma region FPS -Start

    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0;

#pragma endregion

#pragma region Main

    Origindistance01.clear();
    Origindistance02.clear();
    if (UrgNet01 == nullptr && UrgNet02 == nullptr) return;

    std::unique_lock<std::mutex> lockurg(distance_guard);
    if (UrgNet01 != nullptr) {
        if (UrgNet01->recv_distances.size() <= 0) return;
        Origindistance01 = UrgNet01->recv_distances;
    }
    if (UrgNet02 != nullptr) {
        if (UrgNet02->recv_distances.size() <= 0) return;
        Origindistance02 = UrgNet02->recv_distances;
    }
    lockurg.unlock();

    if (ui->Usefilter->isChecked()) {
        //SmoothRealtime(Origindistance01, Previewdistance01, UrgDetector->parm.alldistanceSmoothfactor, UrgDetector->parm.alldistanceSmoothThreshold);
        KalmanF->Filter(Origindistance01);
        if (UrgNet02 != nullptr) {
            //SmoothRealtime(Origindistance02, Previewdistance02, UrgDetector->parm.alldistanceSmoothfactor, UrgDetector->parm.alldistanceSmoothThreshold);
            KalmanF->Filter(Origindistance02);
        }
    }

    if (UrgNet01 != nullptr && UrgNet02 != nullptr) {
        auto regions = UrgDetector->parm.detctRect.slice(RegionInverse);
        auto resultRawObjs_part1 = std::move(UrgDetector->DetectRawObjects(Origindistance01, regions[0]));
        auto resultRawObjs_part2 = std::move(UrgDetector->DetectRawObjects(Origindistance02, regions[1], true));
        resultRawObjs_part1.insert(resultRawObjs_part1.end(), resultRawObjs_part2.begin(), resultRawObjs_part2.end());
        UrgDetector->ProcessingObjects(resultRawObjs_part1);
    }
    else {
        if (UrgNet01 != nullptr) {
            auto resultRawObjs = std::move(UrgDetector->DetectRawObjects(Origindistance01, UrgDetector->parm.detctRect));
            UrgDetector->ProcessingObjects(resultRawObjs);
        }
        else if (UrgNet02 != nullptr) {
            auto resultRawObjs = std::move(UrgDetector->DetectRawObjects(Origindistance02, UrgDetector->parm.detctRect, true));
            UrgDetector->ProcessingObjects(resultRawObjs);
        }
    }

    const std::vector<RawObject>& rawObjectList         = UrgDetector->GetRawObjectList();
    const std::vector<ProcessedObject>& detectedObjects = UrgDetector->GetProcessObjects();

#pragma endregion

#pragma region Draw

    if (UrgNet01 != nullptr || UrgNet02 != nullptr) {
        clearAllPlotData();
        setAllPlotData();

        //Draw Origindistance01
        if (ui->DrawPoint01->isChecked() && !Origindistance01.empty()) {
            PointX01.append(0);
            PointY01.append(0);
            for (int i = 0; i < Origindistance01.size(); i++) {
                vector3 result = Directions[i] * Origindistance01[i];
                PointX01.append(static_cast<double>(result.x));
                PointY01.append(static_cast<double>(result.y));
            }
            PointX01.append(0);
            PointY01.append(0);
            Curveitem01->setData(PointX01, PointY01);
        }

        //Draw Origindistance02
        if (ui->DrawPoint02->isChecked() && !Origindistance02.empty()) {
            PointX02.append(static_cast<double>(UrgDetector->parm.sensor02_originPos.x));
            PointY02.append(0);
            for (int i = 0; i < Origindistance02.size(); i++) {
                vector3 result = Directions[i] * Origindistance02[i] + UrgDetector->parm.sensor02_originPos;
                PointX02.append(static_cast<double>(result.x));
                PointY02.append(static_cast<double>(result.y));
            }
            PointX02.append(static_cast<double>(UrgDetector->parm.sensor02_originPos.x));
            PointY02.append(0);
            Curveitem02->setData(PointX02, PointY02);
        }

        //Draw RawObject
        if (ui->DrawObject->isChecked() && !rawObjectList.empty()) {
            for (auto rObj : rawObjectList) {
                auto rawpos = rObj.getPosition();
                RawObjX.append(static_cast<double>(rawpos.x));
                RawObjY.append(static_cast<double>(rawpos.y));

                for (size_t i = 0; i < rObj.distList.size(); i++) {
                    auto detectpos = rObj.posList[i];
                    ObjPointX.append(static_cast<double>(detectpos.x));
                    ObjPointY.append(static_cast<double>(detectpos.y));
                }
            }
            ui->plot->graph(0)->setData(RawObjX, RawObjY, true);
            ui->plot->graph(2)->setData(ObjPointX, ObjPointY, true);
        }

        //Draw ProcessedObject
        if (UrgNet01 != nullptr || UrgNet02 != nullptr) {
            if (ui->DrawProObject->isChecked() && !detectedObjects.empty()) {
                for (auto& pObj : detectedObjects) {
                    auto propos = pObj.getPosition();
                    PosObjX.append(static_cast<double>(propos.x));
                    PosObjY.append(static_cast<double>(propos.y));
                }
                ui->plot->graph(1)->setData(PosObjX, PosObjY, true);
            }
        }
        ui->plot->replot();
    } 
#pragma endregion

#pragma region Warning

    if (UrgNet01 != nullptr) {
        if (!UrgNet01->GetConnectState()) {
            DisconnectTcp01_Button();
            QMessageBox::information(this, tr("[Sensor01] Connection failed"), tr("Socket failed.\nPlease check if the IP you entered is correct!"));
        }
    }
    if (UrgNet02 != nullptr) {
        if (!UrgNet02->GetConnectState()) {
            DisconnectTcp02_Button();
            QMessageBox::information(this, tr("[Sensor02] Connection failed."), tr("Socket failed.\nPlease check if the IP you entered is correct!"));
        }
    }

#pragma endregion

#pragma region FPS -End

    static double lastFpsKey = 0;
    static int frameCount;
    ++frameCount;

    if (key - lastFpsKey > 2) {
        double FPS = frameCount / (key - lastFpsKey);
        FPSItem->setText(QString("FPS: %1").arg(FPS, 0, 'f', 0));
        UrgDetector->parm.delatime = 1 / FPS;

        lastFpsKey = key;
        frameCount = 0;
    }

#pragma endregion
}

#pragma endregion


#pragma region Destructor & Close Events

QT_URG::~QT_URG() {
    delete ui;
    UrgDetector.reset(nullptr);
    UrgNet01.reset(nullptr);
    UrgNet02.reset(nullptr);
    UrgMouse.reset(nullptr);
    UrgKeyBoard.reset(nullptr);
}

void QT_URG::closeEvent(QCloseEvent* event) {
    ConfigManager::Instance()->SaveWindowSize(this);
    ConfigManager::Instance()->SaveParameter(ui);
    QWidget::closeEvent(event);
}

#pragma endregion

#pragma region Button Event

void QT_URG::ConnectTcp01_Button() {
    if (UrgNet01 == nullptr) {
        UrgNet01.reset(new EthernetConnector(ui->IP_Input_01->text().toStdString(), ui->Port_number_input_01->value(), std::ref(distance_guard)));

        if (UrgNet01->StartTCP()) {
            UrgNet01->StartMeasureDistance();

            drawLabel(LabelItem01, 0, 0, "LiDAR 01", 12, Qt::black);

            ui->Connect_Button_01->setEnabled(false);   
            ui->Disconnect_Button_01->setEnabled(true);
        }
        else {
            UrgNet01.reset(nullptr);
            QMessageBox::information(this, tr("[Sensor01] Connection failed"), tr("Socket failed.\nPlease check if the IP you entered is correct!"));
        }
    }
}

void QT_URG::DisconnectTcp01_Button() {
    if (UrgNet01 != nullptr) {
        UrgNet01.reset(nullptr);
        ui->Connect_Button_01->setEnabled(true);
        ui->Disconnect_Button_01->setEnabled(false);

        clearAllPlotData();
        ui->plot->removeItem(LabelItem01);
        setAllPlotData();
        ui->plot->replot();
    }
}

void QT_URG::ConnectTcp02_Button() {
    if (UrgNet02 == nullptr) {
        UrgNet02.reset(new EthernetConnector(ui->IP_Input_02->text().toStdString(), ui->Port_number_input_02->value(), std::ref(distance_guard)));

        if (UrgNet02->StartTCP()) {
            UrgDetector->parm.sensor02_originPos.x = ui->OriginX->value();
            if (ui->OriginX->value() < 0) RegionInverse = true;
            UrgNet02->StartMeasureDistance();

            drawLabel(LabelItem02, UrgDetector->parm.sensor02_originPos.x, UrgDetector->parm.sensor02_originPos.y, "LiDAR 02", 12, LabelColor);

            ui->Connect_Button_02->setEnabled(false);
            ui->Disconnect_Button_02->setEnabled(true);
        }
        else {
            UrgNet02.reset(nullptr);
            QMessageBox::information(this, tr("[Sensor02] Connection failed."), tr("Socket failed.\nPlease check if the IP you entered is correct!"));
        }
    }
}

void QT_URG::DisconnectTcp02_Button() {
    if (UrgNet02 != nullptr) {
        UrgNet02.reset(nullptr);
        ui->Connect_Button_02->setEnabled(true);
        ui->Disconnect_Button_02->setEnabled(false);

        clearAllPlotData();
        ui->plot->removeItem(LabelItem02);
        setAllPlotData();
        ui->plot->replot();
    }
}

void QT_URG::setParm_Buttom() {
    UrgDetector->parm.noiseLimit            = ui->noiseLimit_Input->value();
    UrgDetector->parm.deltaLimit            = ui->deltaLimit_Input->value();
    UrgDetector->parm.distanceThreshold     = ui->distanceThreshold_Input->value();
    UrgDetector->parm.detectsize            = ui->DetectSize_Input->value();
    UrgDetector->parm.proObjSmoothTime      = ui->SmoothFactor_Input->value();
}

#pragma endregion

#pragma region Plot Methods

void QT_URG::InitPlot() {
    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->setInteraction(QCP::iRangeZoom, true);
    ui->plot->setPlottingHint(QCP::phFastPolylines);

    ui->plot->xAxis->setRange(-2000, 2000);
    ui->plot->yAxis->setRange(-2000, 2000);
    ui->plot->xAxis->setLabel("X-Axes");
    ui->plot->yAxis->setLabel("Y-Axes");

    PointX01.reserve(2162);
    PointX01.reserve(2162);
    PointX02.reserve(2162);
    PointX02.reserve(2162);
    Origindistance01.reserve(2162);
    Origindistance02.reserve(2162);
    Previewdistance01.reserve(2162);
    Previewdistance02.reserve(2162);

    //FPS Label
    FPSItem = new QCPItemText(ui->plot);
    FPSItem->setPositionAlignment(Qt::AlignLeft | Qt::AlignCenter);
    FPSItem->position->setType(QCPItemPosition::ptAxisRectRatio);
    FPSItem->position->setCoords(0.05, 0.02);
    FPSItem->setText(" ");
    FPSItem->setFont(QFont(font().family(), 12));
    FPSItem->setPen(QPen(Qt::transparent));

    Curveitem02 = new QCPCurve(ui->plot->xAxis, ui->plot->yAxis);
    Curveitem02->setPen(QPen(disboarderColor, 2));
    Curveitem02->setBrush(distanceColor02);

    Curveitem01 = new QCPCurve(ui->plot->xAxis, ui->plot->yAxis);
    Curveitem01->setPen(QPen(disboarderColor, 2));
    Curveitem01->setBrush(distanceColor01);

    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssSquare, QPen(objectboarderColor, 3), objectColor, 30));
    ui->plot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsNone);

    ui->plot->addGraph();
    ui->plot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssSquare, QPen(processedObjectboarderColor, 3), processedObjectColor, 30));
    ui->plot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsNone);

    ui->plot->addGraph();
    ui->plot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssPlus, QPen(objectPointColor, 2), objectPointColor, 7));
    ui->plot->graph(2)->setLineStyle(QCPGraph::LineStyle::lsNone);
}

void QT_URG::setAllPlotData() {
    ui->plot->graph(0)->setData(RawObjX,    RawObjY,    true);
    ui->plot->graph(1)->setData(PosObjX,    PosObjY,    true);
    ui->plot->graph(2)->setData(ObjPointX,  ObjPointY,  true);

    Curveitem01->setData(PointX01, PointY01);
    Curveitem02->setData(PointX02, PointY02);
}

void QT_URG::clearAllPlotData() {
    RawObjX.clear();
    RawObjY.clear();
    PosObjX.clear();
    PosObjY.clear();
    PointX01.clear();
    PointY01.clear();
    PointX02.clear();
    PointY02.clear();
    ObjPointX.clear();
    ObjPointY.clear();
}

void QT_URG::drawRect(const Rect& rect, QColor color) {
    if (RectItem == nullptr) {
        RectItem = new QCPItemRect(ui->plot);
        RectItem->setVisible(true);
        RectItem->setPen(QPen(color, 3));
        RectItem->setBrush(QBrush(Qt::NoBrush));
        RectItem->topLeft->setType(QCPItemPosition::ptPlotCoords);
        RectItem->bottomRight->setType(QCPItemPosition::ptPlotCoords);
    }
    RectItem->topLeft->setCoords(rect.x, rect.y);
    RectItem->bottomRight->setCoords(rect.xMax(), rect.yMax());

    ui->plot->replot();
}

void QT_URG::drawLabel(QPointer<QCPItemText>& item, const float x, const float y, const QString& text, int fontsize, QColor color) {
    if (item == nullptr) {
        item = new QCPItemText(ui->plot);
        item->setPositionAlignment(Qt::AlignHCenter | Qt::AlignHCenter);
        item->position->setType(QCPItemPosition::ptPlotCoords);
    }

    item->setFont(QFont(font().family(), fontsize, 5));
    item->setPen(QPen(color, 2));
    item->position->setCoords(x, y);
    item->setText(text);

    ui->plot->replot();
}

#pragma endregion

#pragma region CheckBox EVents

void QT_URG::useOffset(bool checkstate) {
    UrgDetector->parm.useOffset = checkstate;
    if (checkstate) {
        UrgDetector->parm.positionOffset.x = ui->cursoroffsetX_Input->value();
        UrgDetector->parm.positionOffset.y = ui->cursoroffsetY_Input->value();
    }
}

void QT_URG::useTouchEvent(bool checkstate) {
    if (UrgDetector != nullptr) {
        if (checkstate) {
            std::cout << "Start Mouse event" << std::endl;
            UrgDetector->OnNewObjectCallback    = [&]() {UrgMouse->leftDown();};
            UrgDetector->OnLostObjectCallback   = [&]() {UrgMouse->leftUp();};
            UrgDetector->OnUpdataObjCallback    = [&](const vector3& pos) {UrgMouse->move(pos.x, pos.y);};
        }
        else {
            std::cout << "Cancel Mouse event" << std::endl;
            UrgDetector->OnNewObjectCallback    = nullptr;
            UrgDetector->OnLostObjectCallback   = nullptr;
            UrgDetector->OnUpdataObjCallback    = nullptr;
        }
    }
}

void QT_URG::useFlipX(bool checkstate) {
    UrgDetector->parm.useFlipX = checkstate;
}

void QT_URG::useFlipY(bool checkstate) {
    UrgDetector->parm.useFlipY = checkstate;
}

#pragma endregion

#pragma region Spin Events

void QT_URG::setConstraintRegion_Spin() {
    UrgDetector->parm.detctRect.x = -(ui->Width_Input->value() / 2) + ui->OffsetX_Input->value();
    UrgDetector->parm.detctRect.y = ui->Height_Input->value() + ui->OffsetY_Input->value();
    UrgDetector->parm.detctRect.width = ui->Width_Input->value();
    UrgDetector->parm.detctRect.height = ui->Height_Input->value();

    Rect rect = UrgDetector->parm.detctRect;
    drawRect(rect, Qt::red);
}

#pragma endregion

