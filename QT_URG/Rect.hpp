#ifndef ZHI_RECT_H_
#define ZHI_RECT_H_

#include <iostream>
#include <cmath>
#include <vector>
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

	Rect& operator  = (const Rect& r);
	bool Contains(const vector3& point) const;
	std::vector<Rect> slice(bool inverse = false) const;
};
#endif