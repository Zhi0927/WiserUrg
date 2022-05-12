#include "DetectObject.hpp"

RawObject::RawObject()
	: m_guid(GenerateGuid())
{}

RawObject::~RawObject() {}

void RawObject::setPosition(const vector3& value) {
	m_position = value;
}

vector3 RawObject::getPosition(){
	if (!m_positionSet) {
		m_position = dirList[dirList.size() / 2] * distList[distList.size() / 2];
		m_positionSet = true;
	}
	return m_position;
}

float RawObject::getDetectSize(){
	vector3 pointA = dirList[0] * distList[0];
	vector3 pointB = dirList[dirList.size() - 1] * distList[distList.size() - 1];
	float distance = vector3::Distance(pointA, pointB);

	return distance;
}

std::string RawObject::getGuid() const {
	return m_guid;
}



ProcessedObject::ProcessedObject(const vector3& position, const float& size, const float& objectPositionSmoothTime, const float& deltatime)
	:	m_position(position),
		m_detectsize(size),
		m_SmoothTime(objectPositionSmoothTime),
		m_deltaTime(deltatime)
		//m_guid(GenerateGuid())
{
	kalmanV.setfilterValue(position);
}

ProcessedObject::~ProcessedObject() {}

float ProcessedObject::size() {
	return m_detectsize;
}

vector3 ProcessedObject::getPosition() const{
	return m_position;
}

//std::string ProcessedObject::getGuid() const {
//	return m_guid;
//}

bool ProcessedObject::isClear() const {
	return cleared;
}

void ProcessedObject::Update() {
	missingFrame++;
	if (missingFrame >= MISSING_FRAME_LIMIT){
		cleared = true;
	}
}

void ProcessedObject::Update(const vector3 newPos, const float newSize) {
	m_detectsize = newSize;
	m_oldPosition = m_position;

	if (useSmooth) {
		//m_position = kalmanV.kalmanFilterFun(m_position);
		m_position = vector3::SmoothDamp(m_position, newPos, m_currentVelocity, m_SmoothTime, m_deltaTime);
	}
	else{
		m_position = newPos;
	}
	missingFrame = 0;
	m_deltaMovement = m_position - m_oldPosition;
}