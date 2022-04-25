#ifndef HKY_RECT_H_
#define HKY_RECT_H_

#include <iostream>
#include <cmath>

struct alignas(float) Rect {
	float xmin;
	float ymin;
	float width;
	float height;

	Rect();
	Rect(const float rx, const float ry, const float rwidth, const float rheight);
	Rect(const Rect& r);
};
#endif