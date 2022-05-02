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
		m_position = CalcPosition(dirList[dirList.size() / 2], distList[distList.size() / 2]);
		m_positionSet = true;
	}
	return m_position;
}

float RawObject::getDetectSize(){
	vector3 pointA = CalcPosition(dirList[0], distList[0]);
	vector3 pointB = CalcPosition(dirList[dirList.size() -1], distList[distList.size() - 1]);
	float distance = vector3::Distance(pointA, pointB);

	return distance;
}

std::string RawObject::getGuid() const {
	return m_guid;
}

vector3 RawObject::CalcPosition(const vector3& dir, const long& dist) {
	float angle = vector3::Angle(dir, vector3(1, 0, 0));
	float theta = angle * Deg2Rad;
	float x = cos(theta) * dist;
	float y = sin(theta) * dist;

	return vector3(x, y, 0);
}



ProcessedObject::ProcessedObject(const vector3& position, const float& size, const float& objectPositionSmoothTime)
	: m_guid(GenerateGuid()),
	  m_position(position),
	  detectsize(size),
	  m_posSmoothTime(objectPositionSmoothTime),
	  birthTime(clock())
{}

ProcessedObject::~ProcessedObject() {}

float ProcessedObject::size() {
	return detectsize;
}

vector3 ProcessedObject::getPosition() const{
	return m_position;
}

vector3 ProcessedObject::getdeltaMovement() const {
	return m_deltaMovement;
}

std::string ProcessedObject::getGuid() const {
	return m_guid;
}

float ProcessedObject::getage() {
	return clock() - birthTime;
}

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
	detectsize = newSize;
	m_oldPosition = m_position;

	if (useSmooth) {
		m_position = vector3::SmoothDamp(m_position, newPos, m_currentVelocity, m_posSmoothTime);
	}
	else{
		m_position = newPos;
	}
	missingFrame = 0;
	m_deltaMovement = m_position - m_oldPosition;
}