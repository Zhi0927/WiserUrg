#include "DetectObject.hpp"

RawObject::RawObject(const std::vector<vector3>& cachedDirs)
	: m_cachedDirs(cachedDirs),
	  m_guid(GenerateGuid())
{}

RawObject::~RawObject() {}

int RawObject::medianId() {
	return idList[idList.size() / 2];
}
int RawObject::averageId() {
	if (idList.empty()) {
		return 0;
	}
	return static_cast<int>(std::accumulate(idList.begin(), idList.end(), 0) / idList.size());
}

long RawObject::medianDist() {
	return distList[distList.size() / 2];
}
double RawObject::averageDist() {
	if (distList.empty()) {
		return 0;
	}
	return std::accumulate(distList.begin(), distList.end(), 0) / distList.size();
}

float RawObject::getDetectSize() {
	vector3 pointA = CalcPosition(m_cachedDirs[idList[0]], distList[0]);
	vector3 pointB = CalcPosition(m_cachedDirs[idList[idList.size() -1]], distList[distList.size() -1]);
	float distance = vector3::Distance(pointA, pointB);

	return distance;
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
	else
	{
		m_position = newPos;
	}
	missingFrame = 0;
	m_deltaMovement = m_position - m_oldPosition;
}