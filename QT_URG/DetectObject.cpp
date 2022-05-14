#include "DetectObject.hpp"

RawObject::RawObject()
	:	m_guid(GenerateGuid())
{}

RawObject::~RawObject() {}

void RawObject::setPosition(const vector3& value) {
	m_position = value;
}

vector3 RawObject::getPosition(){
	if (!m_positionSet) {
		//m_position = dirList[dirList.size() / 2] * distList[distList.size() / 2];

		m_position = posList[posList.size() / 2];
		m_positionSet = true;
	}
	return m_position;
}

float RawObject::getDetectSize(){
	//vector3 pointA = dirList[0] * distList[0];
	//vector3 pointB = dirList[dirList.size() - 1] * distList[distList.size() - 1];

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
	kalmanV.setfilterValue(position);
}

ProcessedObject::~ProcessedObject() {}


vector3 ProcessedObject::getPosition() const{
	return m_position;
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

void ProcessedObject::Update(const vector3 newPos) {
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


SensedObject::SensedObject(const vector3& vp0, const vector3& vp1, const vector3& vcenter)
	:	p0(vp0),
		p1(vp1),
		center(vcenter),
		m_guid(GenerateGuid())
{}

std::vector<vector3> SensedObject::getVertices() {
	if (m_vertices.size() <= 0)
		m_vertices.resize(5);
	auto width = (p1 - p0).magnitude();
	m_vertices[0] = p0;
	m_vertices[1] = center;
	m_vertices[2] = p1;
	m_vertices[3] = p1 + center.normalize() * width * 0.5f;
	m_vertices[4] = p0 + center.normalize() * width * 0.5f;
	return m_vertices;
}

std::string SensedObject::getGuid() const {
	return m_guid;
}