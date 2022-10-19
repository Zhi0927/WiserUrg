#include "ObjectDetector.h"


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

        float delta1 = abs(distances[i] - distances[i - 1]);
        float delta2 = abs(distances[i + 1] - distances[i]);

        if (region.Contains(pos) && (delta1 < parm.deltaLimit && delta2 < parm.deltaLimit)) {
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

    if (m_processObject.size() != 0){
        for(size_t i =0; i < m_processObject.size(); ++i)
        {
            auto& preObj = m_processObject[i];

            auto comp = [](const RawObject& lhs, const RawObject& rhs) {return lhs.getGuid() < rhs.getGuid(); };
            std::map<RawObject, float, decltype(comp)> objectMapWithDistance;

            for(size_t j = 0; j < newlyDetectedObjects.size(); ++j){
                auto& newObj = newlyDetectedObjects[j];
                float distance = vector3::Distance(newObj.getPosition(), preObj.getPosition());
                objectMapWithDistance[newObj] = distance;
            }

            if (objectMapWithDistance.size() <= 0){
                preObj.Update();
            }
            else{                
                auto closest = std::min_element(objectMapWithDistance.begin(), objectMapWithDistance.end(), [](const auto& l, const auto& r) {return l.second < r.second; });
                if (closest->second <= parm.distanceThreshold){
                    auto temp = std::find_if(newlyDetectedObjects.begin(), newlyDetectedObjects.end(), [&closest](auto& element) { return element == closest->first;});

                    preObj.Update(temp->getPosition());
                    newlyDetectedObjects.erase(temp);

                    if (OnUpdataObjCallback != nullptr) {
                        vector3 pos = preObj.getPosition();
                        SensorPositionNormalize(pos, parm.useFlipX, parm.useFlipY);
                        OnUpdataObjCallback(pos);
                    }
                }
                else{
                    preObj.Update();
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

        for(auto& otherNewObject : newlyDetectedObjects){
            ProcessedObject newProcess(otherNewObject.getPosition(), parm.proObjSmoothTime, parm.delatime);
            m_processObject.emplace_back(newProcess);

            if (OnNewObjectCallback != nullptr && OnUpdataObjCallback != nullptr) {
                vector3 pos = newProcess.getPosition();

                SensorPositionNormalize(pos, parm.useFlipX, parm.useFlipY);
                OnUpdataObjCallback(pos);
                OnNewObjectCallback();
            }
        }
    }
    else {
        for(auto& obj : m_rawObjects){
            ProcessedObject newProcess(obj.getPosition(), parm.proObjSmoothTime, parm.delatime);
            m_processObject.emplace_back(newProcess);

            if (OnNewObjectCallback != nullptr && OnUpdataObjCallback != nullptr) {
                vector3 pos = newProcess.getPosition();

                SensorPositionNormalize(pos, parm.useFlipX, parm.useFlipY);
                OnUpdataObjCallback(pos);
                OnNewObjectCallback();
            }
        }
    }
}

void ObjectDetector::SensorPositionNormalize(vector3& input, bool flipX, bool flipY) {
    input.x = ((input.x - parm.detctRect.x) / parm.detctRect.width);
    input.x = flipX ? input.x : 1 - input.x;

    input.y = -((input.y - parm.detctRect.y) / parm.detctRect.height);
    input.y = flipY ? input.y : 1 - input.y;
}


