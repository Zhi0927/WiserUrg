#ifndef HKY_UTILS_H_
#define HKY_UTILS_H_

#include "vector3d.hpp"

#include "ObjBase.h"
#include <time.h>
#include <string>

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <windows.h>

struct Screen
{	
	int width;
	int height;
};
extern struct Screen screen;



std::vector<long> movingAverages(const std::vector<long>& data, int period);

inline void flipy(vector3& vec);

inline void map(float& value, const float& fromsource, const float& tosource, const float& fromtarget, const float& totarget);

std::string GenerateGuid();

std::string ToString(const int& value, const int pad);

std::vector<std::string> SplitString(const std::string& str);



enum class ZeroPosition {
	LEFT_TOP,
	LEFT_BOTTOM
};
class DataTranslator
{
public:
	DataTranslator(const int& xOffset, const int& yOffset, const int& sensorDetectWidth, const int& sensorDetectHeight);
	~DataTranslator();

	void Sensor2Screen(vector3& inputData, const ZeroPosition zeroPosition = ZeroPosition::LEFT_BOTTOM);

	int m_xOffset;
	int m_yOffest;
	int m_sensorDetectWidth;
	int m_sensorDetectHeight;
};
#endif // !HKY_UTILS_H_

