#include "ObjectDetector.hpp"


ObjectDetector::ObjectDetector(){
    std::cout << "Detector Init" << std::endl;
}

ObjectDetector::~ObjectDetector() {}


const std::vector<vector3>& ObjectDetector::GetDirection() const {
    return m_directions;
}

const std::vector<RawObject>& ObjectDetector::GetRawObjectList() const {
    return m_rawObjects;
}

const std::vector<ProcessedObject>& ObjectDetector::GetProcessObjects() const {
    return m_processObject;
}

void ObjectDetector::CacheDirections(int ScanSteps) {
    float offset = M_DELTA_ANGLE * 540;     
    m_directions.resize(ScanSteps);
    for (size_t i = 0; i < m_directions.size(); i++){
        float a = M_DELTA_ANGLE * i + offset;
        m_directions[i] = vector3(-cos(a), -sin(a), 0); 
    }
    std::cout << "Direction has been calculated!" << std::endl;
}

void ObjectDetector::GetPointFromDistance(int step, float distance, vector3& pos, bool multi){
    pos = m_directions[step] * distance;
    if (multi) {
        pos += parm.sensor02_originPos;
    }
}

std::vector<RawObject> ObjectDetector::DetectRawObjects(const std::vector<long>& distances, const Rect& region, bool subsensor) {
    std::vector<RawObject> rawobjects;

    if (m_directions.size() <= 0) {
        std::cerr << "directions vector is not setup!" << std::endl;
        return rawobjects;
    }
    
    bool isGrouping = false;
    for (size_t i = 1; i < distances.size() - 1; i++) {

        auto pos = m_directions[i] * distances[i];
        if (subsensor) {
            pos += parm.sensor02_originPos;
        }

        if (!region.Contains(pos)) continue;

        float deltaA = abs(distances[i] - distances[i - 1]);
        float deltaB = abs(distances[i + 1] - distances[i]);

        if (deltaA < parm.deltaLimit && deltaB < parm.deltaLimit) {
            if (!isGrouping) {
                isGrouping = true;

                RawObject newObject;
                newObject.distList.emplace_back(distances[i]);
                newObject.posList.emplace_back(pos);
                rawobjects.emplace_back(newObject);
            }
            else {
                auto& newObject = rawobjects[rawobjects.size() - 1];

                newObject.distList.emplace_back(distances[i]);
                newObject.posList.emplace_back(pos);
            }
        }
        else {
            if (isGrouping) {
                isGrouping = false;
            }
        }
    }

    for (int i = 0; i < rawobjects.size(); ++i) {
        if (rawobjects[i].posList.size() < parm.noiseLimit || rawobjects[i].getDetectSize() > parm.detectsize) {
            rawobjects.erase(rawobjects.begin() + i);
            --i;
        }
        else {
            rawobjects[i].posList = movingAverages(rawobjects[i].posList, 5);
        }
    }

    return rawobjects;
}

void ObjectDetector::ProcessingObjects(std::vector<RawObject>& newlyDetectedObjects) {
    m_rawObjects.clear();

    if (parm.useOffset){
        for(auto& obj : newlyDetectedObjects){
            obj.setPosition(obj.getPosition() += parm.positionOffset);
        }
    }

    m_rawObjects = newlyDetectedObjects;

    //std::unique_lock<std::mutex> lockdata(m_detectobject_guard);

    if (m_processObject.size() != 0){
        for(size_t i =0; i < m_processObject.size(); ++i)
        {
            auto& oldObj = m_processObject[i];
            std::map<std::string, float> objectMapWithDistance;

            for(size_t j = 0; j < newlyDetectedObjects.size(); ++j){
                auto& newObj = newlyDetectedObjects[j];
                float distance = vector3::Distance(newObj.getPosition(), oldObj.getPosition());
                objectMapWithDistance[newObj.getGuid()] = distance;
            }

            if (objectMapWithDistance.size() <= 0){
                oldObj.Update();
            }
            else{                
                auto closest = std::min_element(objectMapWithDistance.begin(), objectMapWithDistance.end(), [](const auto& l, const auto& r) {return l.second < r.second; });
                if (closest->second <= parm.distanceThreshold){
                    auto temp = std::find_if(newlyDetectedObjects.begin(), newlyDetectedObjects.end(), [&closest](auto& ele) { return ele.getGuid() == closest->first;});
                    oldObj.Update(temp->getPosition());
                    newlyDetectedObjects.erase(temp);

                    if (OnUpdataObjCallback != nullptr) {
                        vector3 pos = oldObj.getPosition();
                        Sensor2Screen(pos);
                        OnUpdataObjCallback(pos);
                    }
                }
                else{
                    oldObj.Update();
                }
            }
        }


        for (int i = 0; i < m_processObject.size(); ++i){
            if (m_processObject[i].isClear()){         
                m_processObject.erase(m_processObject.begin() + i);
                --i;

                if (OnLostObjectCallback != nullptr) {
                    OnLostObjectCallback();
                }
            }
        }
        for(auto& leftOverNewObject : newlyDetectedObjects){
            ProcessedObject newbie(leftOverNewObject.getPosition(), parm.objPosSmoothTime, parm.delatime);
            m_processObject.emplace_back(newbie);

            if (OnNewObjectCallback != nullptr && OnUpdataObjCallback != nullptr) {
                vector3 pos = newbie.getPosition();
                Sensor2Screen(pos);
                OnUpdataObjCallback(pos);
                OnNewObjectCallback();
            }

        }
    }
    else {
        for(auto& obj : m_rawObjects){
            ProcessedObject newbie(obj.getPosition(), parm.objPosSmoothTime, parm.delatime);
            m_processObject.emplace_back(newbie);

            if (OnNewObjectCallback != nullptr && OnUpdataObjCallback != nullptr) {
                vector3 pos = newbie.getPosition();
                Sensor2Screen(pos);
                OnUpdataObjCallback(pos);
                OnNewObjectCallback();
            }
        }
    }
}

void ObjectDetector::Sensor2Screen(vector3& input) {

    input.x = (1- ((input.x - parm.detctRect.xmin) / parm.detctRect.width)) * parm.screenWidth;
    input.y = (1+ ((input.y - parm.detctRect.ymin) / parm.detctRect.height)) * parm.screenHeight;
}


