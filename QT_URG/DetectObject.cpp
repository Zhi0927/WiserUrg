#include "DetectObject.h"

RawObject::RawObject()
	:	m_guid(GenerateGuid())
{}

RawObject::~RawObject() {}

void RawObject::setPosition(const vector3& value) {
	m_position = value;
}

vector3 RawObject::getPosition(){
	if (!m_positionSet) {
		m_position = posList[posList.size() / 2];
		m_positionSet = true;
	}
	return m_position;
}

bool RawObject::operator < (const RawObject& rhs) const
{
	return m_guid < rhs.getGuid();
}

bool RawObject::operator == (const RawObject& rhs) const {
	return m_guid == rhs.getGuid();
}

float RawObject::getDetectSize(){
	vector3 pointA = posList[0];
	vector3 pointB = posList[posList.size() - 1];;
	float distance = vector3::Distance(pointA, pointB);

	return distance;
}

std::string RawObject::getGuid() const {
	return m_guid;
}



ProcessedObject::ProcessedObject(const vector3& position, const float& objectPositionSmoothTime, const float& deltatime)
		:	m_position(position),
			m_SmoothTime(objectPositionSmoothTime),
			m_deltaTime(deltatime)
{
	//kalmanV.setfilterValue(position);
}

ProcessedObject::~ProcessedObject() {}


vector3 ProcessedObject::getPosition() const{
	return m_position;
}

bool ProcessedObject::isClear() const {
	return m_cleared;
}

void ProcessedObject::Update() {
	m_missingFrame++;
	if (m_missingFrame >= MISSING_FRAME_LIMIT){
		m_cleared = true;
	}
}

void ProcessedObject::Update(const vector3 newPos) {
	m_oldPosition = m_position;

	if (useSmooth) {
		//m_position = kalmanV.kalmanFilterFun(m_position);
		m_position = vector3::SmoothDamp(m_position, newPos, m_currentVelocity, m_SmoothTime, m_deltaTime);
	}
	else{
		m_position = newPos;
	}
	m_missingFrame = 0;
	m_deltaMovement = m_position - m_oldPosition;
}