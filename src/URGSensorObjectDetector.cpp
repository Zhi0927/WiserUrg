#include "URGSensorObjectDetector.hpp"


URGSensorObjectDetector::URGSensorObjectDetector(const std::string& ip, const int& port)
    :m_ip_address(ip), m_port_number(port)
{
    m_urg.reset(new UrgDeviceEthernet(m_ip_address, m_port_number));
    m_urg->StartTCP();

    StartMeasureDistance();
}

URGSensorObjectDetector::~URGSensorObjectDetector() {
    m_urg.reset();
}


Rect URGSensorObjectDetector::detectAreaRect() {
	Rect rect(0, 0, m_detectRectWidth, m_detectRectHeight);
	rect.xmin -= (m_detectRectWidth / 2);
	return rect;
}

ProcessedObject* URGSensorObjectDetector::GetProcessedObjectByGuid(const std::string& guid) {

    for(ProcessedObject obj : m_detectedObjects)
    {
        if (obj.getGuid() == guid)
        {
            return &obj;
        }
    }
    std::cerr << "[ERROR] cannot find processed object with guid " << guid << std::endl;
    return nullptr;
}

RawObject* URGSensorObjectDetector::GetRawObjectByGuid(const std::vector<RawObject>& rawobjects, const std::string& guid) {

    for (RawObject obj : rawobjects)
    {
        if (obj.getGuid() == guid)
        {
            return &obj;
        }
    }
    std::cerr << "cannot find raw object with guid " << guid << std::endl;
    return nullptr;
}

std::vector<ProcessedObject> URGSensorObjectDetector::GetObjects(const std::vector<ProcessedObject>& detectobj, const float ageFilter) {

    std::vector<ProcessedObject> target(detectobj);
    for (int i = 0; i < target.size(); ++i) {
        if (target[i].getage() < ageFilter) {
            target.erase(target.begin() + i);
            --i;
        }
    }
    return target;
}

void URGSensorObjectDetector::CalculateDistanceConstrainList(const int steps) {
    switch (m_distanceCroppingMethod)
    {
    case DistanceCroppingMethod::RADIUS:
        for (size_t i = 0; i < steps; i++) {
            if (m_directions[i].y < 0) {
                m_distanceConstrainList[i] = 0;
            }
            else
            {
                m_distanceConstrainList[i] = m_maxDetectionDist;
            }
        }
        break;

    case DistanceCroppingMethod::RECT:
        float keyAngle = atan(m_detectRectHeight / (m_detectRectWidth / 2.f));

        for (size_t i = 0; i < steps; i++) {
            if (m_directions[i].y <= 0) {
                m_distanceConstrainList[i] = 0;
            }
            else {
                float a = vector3::Angle(m_directions[i], vector3(1, 0, 0)) * Deg2Rad;
                float tanAngle = tan(a);
                float pn = tanAngle / abs(tanAngle);

                float r = 0;
                if (a < keyAngle || a > M_PI - keyAngle) {
                    float x = pn * m_detectRectWidth / 2;
                    float y = x * tan(a);
                    r = y / sin(a);
                }
                else if (a >= keyAngle && a <= M_PI - keyAngle) {
                    float angle2 = M_PI / 2 - a;
                    float y = m_detectRectHeight;
                    float x = y * tan(angle2);
                    r = x / sin(angle2);
                }

                if (r < 0 || std::isnan(r)) {
                    r = 0;
                }

                m_distanceConstrainList[i] = static_cast<long>(r);
            }
        }
        break;
    }
}

std::vector<long> URGSensorObjectDetector::ConstrainDetectionArea(const std::vector<long>& beforeCrop, DistanceCroppingMethod method) {
    std::vector<long> result;
    for (size_t i = 0; i < beforeCrop.size(); i++) {
        if (beforeCrop[i] > m_distanceConstrainList[i] || beforeCrop[i] <= 0) {
            result.emplace_back(m_distanceConstrainList[i]);
        }
        else {
            result.emplace_back(beforeCrop[i]);
        }
    }
    return result;
}

void URGSensorObjectDetector::StartMeasureDistance() {
    m_urg->Write(SCIP_Writer::MD(0, 1080, 1, 0, 0));
}

void URGSensorObjectDetector::CacheDirections() {
    float d = M_PI * 2 / 1440;
    float offset = d * 540;
    m_directions.resize(m_sensorScanSteps);
    for (size_t i = 0; i < m_directions.size(); i++)
    {
        float a = d * i + offset;
        m_directions[i] = vector3(-cos(a), -sin(a), 0);
    }
}

std::vector<long> URGSensorObjectDetector::SmoothDistanceCurve(const std::vector<long>& croppedDistances, int smoothKernelSize) {
    return movingAverages(croppedDistances, smoothKernelSize);
}

std::vector<RawObject> URGSensorObjectDetector::DetectObjects(const std::vector<long>& croppedDistances, const std::vector<long>& distanceConstrainList) {
    if (m_directions.size() <= 0)
    {
        std::cerr << "directions vector is not setup." << std::endl;
        return std::vector<RawObject>();
    }

    std::vector<RawObject> resultList;
    bool isGrouping = false;
    for (int i = 1; i < croppedDistances.size() - 1; i++)
    {

        float deltaA = abs(croppedDistances[i] - croppedDistances[i - 1]);
        float deltaB = abs(croppedDistances[i + 1] - croppedDistances[i]);

        auto dist   = croppedDistances[i];
        auto ubDist = distanceConstrainList[i];

        if (dist < ubDist && (deltaA < m_deltaLimit && deltaB < m_deltaLimit))
        {
            if (!isGrouping)
            {
                isGrouping = true;
                RawObject newObject(m_directions);
                newObject.m_idList.emplace_back(i);
                newObject.m_distList.emplace_back(dist);
                resultList.emplace_back(newObject);
            }
            else
            {
                auto newObject = resultList[resultList.size() - 1];
                newObject.m_idList.emplace_back(i);
                newObject.m_distList.emplace_back(dist);
            }
        }
        else
        {
            if (isGrouping)
            {
                isGrouping = false;
            }
        }
    }

    for (int i = 0; i < resultList.size(); i++) {
        if (resultList[i].m_idList.size() < m_noiseLimit)
        {
            m_detectedObjects.erase(m_detectedObjects.begin() + i);
            --i;
        }
    }

    for (auto item : resultList)  item.GetCalcPosition();

    return resultList;
}

void URGSensorObjectDetector::UpdateObjectList() {
    std::vector<RawObject> newlyDetectedObjects(DetectObjects(m_croppedDistances, m_distanceConstrainList));

    if (m_useOffset)
    {
        for(auto obj : newlyDetectedObjects)
        {
            obj.setPosition(obj.getPosition() += m_positionOffset);
        }
    }

    m_rawObjectList.assign(newlyDetectedObjects.begin(), newlyDetectedObjects.end());


    std::unique_lock<std::mutex> lockdata(m_detectobject_guard);

    if (m_detectedObjects.size() != 0)
    {
        for(size_t i =0; i < m_detectedObjects.size(); ++i)
        {
            auto& oldObj = m_detectedObjects[i];

            std::map<std::string, float> objectMapWithDistance;

            for(size_t j = 0; j < newlyDetectedObjects.size(); ++j)
            {
                auto& newObj = newlyDetectedObjects[j];
                float distance = vector3::Distance(newObj.getPosition(), oldObj.getPosition());

                objectMapWithDistance[newObj.getGuid()] = distance;
            }

            if (objectMapWithDistance.size() <= 0)
            {
                oldObj.Update();
            }
            else
            {                
                auto closest = std::min_element(objectMapWithDistance.begin(), objectMapWithDistance.end(), [](const auto& l, const auto& r) {return l.second < r.second; });
                if (closest->second <= m_distanceThresholdForMerge)
                {
                    RawObject* temp = GetRawObjectByGuid(newlyDetectedObjects, closest->first);
                    if (temp != nullptr) {
                        oldObj.Update(temp->getPosition(), temp->size());

                        newlyDetectedObjects.erase(std::remove_if(newlyDetectedObjects.begin(), newlyDetectedObjects.end(), [&temp](const RawObject& ele) {
                            return ele.getGuid() == temp->getGuid();
                            }), newlyDetectedObjects.end());
                    }
                }
                else
                {
                    oldObj.Update();
                }
            }
        }

        for (int i = 0; i < m_detectedObjects.size(); ++i)
        {
            if (m_detectedObjects[i].isClear())
            {
                if (m_OnLostObject != nullptr) { m_OnLostObject(m_detectedObjects[i]); }

                m_detectedObjects.erase(m_detectedObjects.begin() + i);
                --i;
            }
        }

        for(auto leftOverNewObject : newlyDetectedObjects)
        {
            ProcessedObject newbie(leftOverNewObject.getPosition(), leftOverNewObject.size(), m_objectPositionSmoothTime);
            m_detectedObjects.emplace_back(newbie);

            if (m_OnNewObject != nullptr) { m_OnNewObject(newbie); }
        }
    }
    else 
    {
        for(auto obj : m_rawObjectList)
        {
            ProcessedObject newbie(obj.getPosition(), obj.size(), m_objectPositionSmoothTime);
            m_detectedObjects.emplace_back(newbie);

            if (m_OnNewObject != nullptr) { m_OnNewObject(newbie); }
        }
    }   
}

std::vector<long> URGSensorObjectDetector::SmoothDistanceCurveByTime(const std::vector<long>& newList, std::vector<long>& previousList, float smoothFactor) {
    if (previousList.size() <= 0)
    {
        previousList.assign(newList.begin(), newList.end());
        return newList;
    }
    else
    {
        std::vector<long> result(newList.size());
        for (size_t i = 0; i < result.size(); i++)
        {

            float diff = newList[i] - previousList[i];
            if (diff > m_timeSmoothBreakingDistanceChange)
            {
                result[i] = newList[i];
                previousList[i] = result[i];
            }
            else
            {
                float smallDiff = diff * smoothFactor;
                float final = previousList[i] + smallDiff;

                result[i] = static_cast<long>(final);
                previousList[i] = result[i];
            }
        }
        return result;
    }
}

void URGSensorObjectDetector::mainloop() {

    if (m_smoothKernelSize % 2 == 0) {
        m_smoothKernelSize += 1;
    }

    std::vector<long> originalDistances;

    std::unique_lock<std::mutex> lockurg(m_urgdistance_guard);

    if (m_urg->m_distances.size() <= 0) return;
    originalDistances.assign(m_urg->m_distances.begin(), m_urg->m_distances.end());

    lockurg.unlock();

    if (originalDistances.size() <= 0) return;


    //Setting up things, one time
    if (m_sensorScanSteps <= 0){
        m_sensorScanSteps = m_urg->m_distances.size();
        m_distanceConstrainList.reserve(m_sensorScanSteps);
        CacheDirections();
        CalculateDistanceConstrainList(m_sensorScanSteps);
    }

    if (m_recalculateConstrainAreaEveryFrame){
        if (m_frameTime % 10 == 0){
            CalculateDistanceConstrainList(m_sensorScanSteps);
        }
    }

    if (m_gd_loop){
        m_urg->Write(SCIP_Writer::GD(0, 1080));
    }

    auto cropped = ConstrainDetectionArea(originalDistances, m_distanceCroppingMethod);
    m_croppedDistances.clear();
    m_croppedDistances.insert(m_croppedDistances.end(), cropped.begin(), cropped.end());


    if (m_smoothDistanceCurve){
        m_croppedDistances = SmoothDistanceCurve(m_croppedDistances, m_smoothKernelSize);
    }
    if (m_smoothDistanceByTime){
        m_croppedDistances = SmoothDistanceCurveByTime(m_croppedDistances, m_smoothByTimePreviousList, m_timeSmoothFactor);
    }

    UpdateObjectList();

    m_frameTime+= 1;
}