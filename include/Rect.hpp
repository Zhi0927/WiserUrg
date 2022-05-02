#ifndef HKY_RECT_H_
#define HKY_RECT_H_

#include <iostream>
#include <cmath>
#include "Vector3.hpp"

struct alignas(float) Rect {

public:
	float xmin;
	float ymin;
	float width;
	float height;

public:
	Rect();
	Rect(const float rx, const float ry, const float rwidth, const float rheight);
	Rect(const Rect& r);

	float xmax() const;
	float ymax() const;

	bool Contains(vector3 point);

	Rect& operator  = (const Rect& r);
};
#endif