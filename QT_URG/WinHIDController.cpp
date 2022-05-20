#include "WinHIDController.hpp"


void MouseSimulator::SetRatio(int screenWidth, int screenHeight) {
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}

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


HotkeyManager::~HotkeyManager() {
	close();
}


void HotkeyManager::Init(std::function<void(uint32_t, uint32_t, bool, bool)> onCallback) {
	if (isdetect)
		return;
	m_callback = onCallback;

	isdetect = true;
	m_thread.reset(new std::thread(&HotkeyManager::runner_thread, this));
}

void HotkeyManager::setCallback(std::function<void(uint32_t, uint32_t, bool, bool)> onCallback) {
	m_callback = onCallback;
}

bool HotkeyManager::isCtrlPressed() {
	return 0 != (GetKeyState(VK_CONTROL) & 0x8000);
}

bool HotkeyManager::isShiftPressed() {
	return 0 != (GetKeyState(VK_SHIFT) & 0x8000);
}

uint32_t HotkeyManager::getProcessId() {
	HWND hWnd = GetForegroundWindow();
	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	return dwProcessId;
}

void HotkeyManager::runner_thread() {
	while (isdetect) {
		for (int key_id = 0x30; key_id <= 0x5A; key_id++) {
			if (VK_CONTROL == key_id || VK_SHIFT == key_id)
				continue;

			if (!GetAsyncKeyState(key_id))
				continue;

			if (m_callback == nullptr)
				continue;

			m_callback(getProcessId(), key_id, isCtrlPressed(), isShiftPressed());
		}
		Sleep(120);
	}
}

void HotkeyManager::close() {
	isdetect = false;
	if (m_thread != nullptr) {
		if (m_thread->joinable()) {
			m_thread->join();
		}
		m_thread.reset(nullptr);
	}
}