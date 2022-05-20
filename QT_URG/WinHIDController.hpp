#ifndef ZHI_MOUSESIMULATOR_H_
#define ZHI_MOUSESIMULATOR_H_

#include <iostream>
#include <functional>
#include <thread>
#include <windows.h>

class MouseSimulator
{
public:
	static MouseSimulator* Instance() {
		static MouseSimulator singleton;
		return &singleton;
	}

	void SetRatio(int screenWidth, int screenHeight);
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


class HotkeyManager {
public:

	static HotkeyManager* Instance() {
		static HotkeyManager singleton;
		return &singleton;
	}

	~HotkeyManager();
	void Init(std::function<void(uint32_t, uint32_t, bool, bool)> onCallback);
	void setCallback(std::function<void(uint32_t, uint32_t, bool, bool)> onCallback);

private:
	bool isCtrlPressed();
	bool isShiftPressed();
	uint32_t getProcessId();
	void runner_thread();
	void close();

private:
	std::function<void(uint32_t, uint32_t, bool, bool)> m_callback;
	std::unique_ptr<std::thread>						m_thread;
	bool												isdetect = false;
};
#endif
