#pragma once

#include <Windows.h>

#include "VulkanUtil.h"

class Window
{
public:
	Window(int width, int height);

	int MainLoop();

	std::pair<int, int> GetSize() const
	{
		return std::pair<int, int>(m_width, m_height);
	}

	HWND GetHWnd() const 
	{ 
		return m_hWnd; 
	}

	Signal<> OnTick;
	Signal<> OnSuspend;
	Signal<> OnResume;
	Signal<int, int> OnResize;

private:
	int m_width;
	int m_height;
	HWND m_hWnd;
	// Raw Input buffer
	std::vector<char> m_buf;

	static constexpr const wchar_t* WINDOW_CLASS = L"MyWindowClass";

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};