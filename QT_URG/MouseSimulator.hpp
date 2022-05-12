#ifndef ZHI_MOUSESIMULATOR_H_
#define ZHI_MOUSESIMULATOR_H_

#include <iostream>
#include <windows.h>

class MouseSimulator
{
public:
	MouseSimulator(int screenWidth, int screenHeight);

	void move(float dx, float dy);
	void rightDown();
	void rightUp();
	void leftDown();
	void leftUp();
	void leftClick();

private:
	void input(DWORD dwFlags, LONG dx = 0, LONG dy = 0, DWORD mouseData = 0, DWORD time = 0, ULONG_PTR dwExtraInfo = 0);

private:
	int m_screenWidth	= 1920;
	int m_screenHeight	= 1080;
};
#endif
