#pragma once

#include <Windows.h>

#include "VulkanUtil.h"

class Window
{
public:
	Window(int width, int height, const std::wstring& title);

	int MainLoop();

	std::pair<int, int> GetSize() const
	{
		return std::pair<int, int>(m_width, m_height);
	}

	HWND GetHWnd() const 
	{ 
		return m_hWnd; 
	}

	void SetTitle(const std::string& text)
	{
		SetWindowTextA(m_hWnd, text.c_str());
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

	static constexpr const wchar_t* WINDOW_CLASS = L"WindowClass";

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};