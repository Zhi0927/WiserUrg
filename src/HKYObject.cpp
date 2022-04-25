#include "HKYObject.hpp"

RawObject::RawObject(const std::vector<vector3>& cachedDirs)
	: m_cachedDirs(cachedDirs),
	  m_guid(GenerateGuid())
{}

RawObject::~RawObject() {}

int RawObject::medianId() {
	return m_idList[m_idList.size() / 2];
}
int RawObject::averageId() {
	if (m_idList.empty()) {
		return 0;
	}
	return static_cast<int>(std::accumulate(m_idList.begin(), m_idList.end(), 0) / m_idList.size());
}

long RawObject::medianDist() {
	return m_distList[m_distList.size() / 2];
}
double RawObject::averageDist() {
	if (m_distList.empty()) {
		return 0;
	}
	return std::accumulate(m_distList.begin(), m_distList.end(), 0) / m_distList.size();
}

float RawObject::size() {
	vector3 pointA = CalcPosition(m_cachedDirs[m_idList[0]], m_distList[0]);
	vector3 pointB = CalcPosition(m_cachedDirs[m_idList[m_idList.size() -1]], m_distList[m_distList.size() -1]);

	return vector3::Distance(pointA, pointB);
}


void RawObject::setPosition(const vector3& value) {
	m_position = value;
}
vector3 RawObject::getPosition() const {
	if (!m_positionSet) std::cerr << "position has not bee set yet\n"; 	

	return m_position;
}

std::string RawObject::getGuid() const {
	return m_guid;
}

void RawObject::GetCalcPosition() {
	m_position = CalcPosition();
	m_positionSet = true;
}


vector3 RawObject::CalcPosition() {
	long media = medianDist();
	return CalcPosition(m_cachedDirs[medianId()], media);
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
	  m_size(size),
	  m_posSmoothTime(objectPositionSmoothTime),
	  m_birthTime(clock())
{}

ProcessedObject::~ProcessedObject() {}

float ProcessedObject::size() {
	return m_size;
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
	return clock() - m_birthTime;
}

bool ProcessedObject::isClear() const {
	return m_clear;
}

void ProcessedObject::Update() {
	m_missingFrame++;
	if (m_missingFrame >= MISSING_FRAME_LIMIT)
	{
		m_clear = true;
	}
}

void ProcessedObject::Update(const vector3& newPos, const float newSize) {
	m_size = newSize;
	m_oldPosition = m_position;

	if (m_useSmooth) {
		m_position = vector3::SmoothDamp(m_position, newPos, m_currentVelocity, m_posSmoothTime);
	}
	else
	{
		m_position = newPos;
	}
	m_missingFrame = 0;
	m_deltaMovement = m_position - m_oldPosition;
}