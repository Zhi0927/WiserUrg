#include "Rect.hpp"


Rect::Rect() :xmin(0), ymin(0), width(0), height(0){}

Rect::Rect(const float rx, const float ry, const float rwidth, const float rheight)
	:xmin(rx), ymin(ry), width(rwidth), height(rheight){}

Rect::Rect(const Rect& r) 
	: xmin(r.xmin), ymin(r.ymin), width(r.width), height(r.height){}


bool Rect::Contains(vector3 point){
	return (point.x >= xmin) && (point.x < xmax()) && (point.y < ymin) && (point.y > ymax());
}

float Rect::xmax() const{
	return xmin + width;
}
float Rect::ymax() const {
	return ymin - height;
}

Rect& Rect::operator  = (const Rect& r) {
	xmin	= r.xmin;
	ymin	= r.ymin;
	width	= r.width;
	height	= r.height;

	return *this;
}