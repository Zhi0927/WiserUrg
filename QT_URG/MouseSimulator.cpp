#include "MouseSimulator.hpp"

MouseSimulator::MouseSimulator(int screenWidth, int screenHeight) : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {}

void MouseSimulator::move(float dx, float dy)
{
	input(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, dx * 65535.0f / m_screenWidth, dy * 65535.0f / m_screenHeight);
}

void MouseSimulator::rightDown()
{
	input(MOUSEEVENTF_RIGHTDOWN);
}

void MouseSimulator::rightUp()
{
	input(MOUSEEVENTF_RIGHTUP);
}

void MouseSimulator::leftDown()
{
	input(MOUSEEVENTF_LEFTDOWN);
}

void MouseSimulator::leftUp()
{
	input(MOUSEEVENTF_LEFTUP);
}

void MouseSimulator::leftClick() {
	input(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP);
}


void MouseSimulator::input(DWORD dwFlags, LONG dx, LONG dy, DWORD mouseData, DWORD time, ULONG_PTR dwExtraInfo)
{
	INPUT input = { INPUT_MOUSE, dx, dy, mouseData, dwFlags, time, dwExtraInfo };
	::SendInput(1, &input, sizeof(INPUT));
}