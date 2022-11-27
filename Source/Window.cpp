#include "Window.h"

#include "Keyboard.h"
#include "Mouse.h"

namespace
{
	constexpr int GetX(LPARAM lParam)
	{
		return static_cast<int>(static_cast<short>(LOWORD(lParam)));
	}

	constexpr int GetY(LPARAM lParam)
	{
		return static_cast<int>(static_cast<short>(HIWORD(lParam)));
	}
}

Window::Window(int width, int height, const std::wstring& title) :
	m_width(0),
	m_height(0),
	m_hWnd(nullptr)
{
	SetProcessDPIAware();

	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.lpfnWndProc = &WndProc;
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	auto rc = RegisterClassExW(&wcex);
	if (!rc)
	{
		throw std::runtime_error("Error: Failed to register window class");
	}
	
	m_hWnd = CreateWindowExW(0, WINDOW_CLASS, title.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, 
		nullptr, nullptr, nullptr, this);
	if (!m_hWnd)
	{
		throw std::runtime_error("Error: Failed to create window");
	}
	RECT rect{};
	GetClientRect(m_hWnd, &rect);
	m_width = rect.right - rect.left;
	m_height = rect.bottom - rect.top;
	SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Register for Raw Input
	RAWINPUTDEVICE rid{};
	rid.usUsagePage = 1;
	rid.usUsage = 2;
	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
	{
		throw std::runtime_error("Error: Failed to register for Raw Input");
	}
	
	ShowWindow(m_hWnd, SW_SHOW);
}

int Window::MainLoop()
{
	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			OnTick.Send();
		}
	}
	return 0;
}

LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool s_mouseOverWindow = false;
	static bool s_minimized = false;
	static bool s_resizing = false;
	static bool s_fullscreen = false;
	static RECT s_prevRect{};

	auto* pInstance = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (uMsg)
	{
	case WM_DESTROY:
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_PAINT:
	{
		if (s_resizing)
		{
			pInstance->OnTick.Send();
		}
		else
		{
			PAINTSTRUCT ps{};
			BeginPaint(pInstance->m_hWnd, &ps);
			EndPaint(pInstance->m_hWnd, &ps);
		}
		return 0;
	}
	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED && !s_minimized)
		{
			s_minimized = true;
			pInstance->OnSuspend.Send();
		}
		else if (s_minimized)
		{
			s_minimized = false;
			pInstance->OnResume.Send();
		}
		else
		{
			pInstance->m_width = LOWORD(lParam);
			pInstance->m_height = HIWORD(lParam);
			pInstance->OnResize.Send(pInstance->m_width, pInstance->m_height);
		}
		return 0;
	}
	case WM_ENTERSIZEMOVE:
	{
		s_resizing = true;
		return 0;
	}
	case WM_EXITSIZEMOVE:
	{
		s_resizing = false;
		return 0;
	}
	case WM_SYSKEYDOWN:
	{
		// Fullscreen toggle (ALT + ENTER)
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			if (s_fullscreen)
			{
				// Go windowed
				SetWindowLongPtrW(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				SetWindowLongPtrW(hWnd, GWL_EXSTYLE, 0);
				ShowWindow(hWnd, SW_SHOWNORMAL);
				SetWindowPos(hWnd, HWND_TOP, s_prevRect.left, s_prevRect.top,
					s_prevRect.right - s_prevRect.left, s_prevRect.bottom - s_prevRect.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
				s_fullscreen = false;
			}
			else
			{
				// Go fullscreen
				GetWindowRect(hWnd, &s_prevRect);
				SetWindowLongPtrW(hWnd, GWL_STYLE, WS_POPUP);
				SetWindowLongPtrW(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
				s_fullscreen = true;
			}
		}
		return 0;
	}
	case WM_KEYDOWN:
	{
		WORD code = LOWORD(wParam);
		Keyboard::GetInstance().OnKey(IntToEnum<Key>(code), KeyEvent::ePress);
		return 0;
	}
	case WM_KEYUP:
	{
		WORD code = LOWORD(wParam);
		Keyboard::GetInstance().OnKey(IntToEnum<Key>(code), KeyEvent::eRelease);
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		POINT pt{ GetX(lParam), GetY(lParam) };
		Mouse::GetInstance().OnMove(pt.x, pt.y);

		if (!s_mouseOverWindow)
		{
			s_mouseOverWindow = true;
			Mouse::GetInstance().OnEnter();

			TRACKMOUSEEVENT tme{};
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);
		}

		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		Mouse::GetInstance().OnButton(Button::eLeft, ButtonEvent::eClick);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		Mouse::GetInstance().OnButton(Button::eLeft, ButtonEvent::eRelease);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		Mouse::GetInstance().OnButton(Button::eRight, ButtonEvent::eClick);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		Mouse::GetInstance().OnButton(Button::eRight, ButtonEvent::eRelease);
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);
		Mouse::GetInstance().OnScroll(delta);
		return 0;
	}
	case WM_MOUSELEAVE:
	{
		s_mouseOverWindow = false;
		Mouse::GetInstance().OnLeave();
		return 0;
	}
	case WM_INPUT:
	{
		UINT size;
		UINT rc = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, 
			nullptr, &size, sizeof(RAWINPUTHEADER));
		if (rc != 0)
		{
			return 0;
		}
		pInstance->m_buf.resize(size);

		rc = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
			pInstance->m_buf.data(), &size, sizeof(RAWINPUTHEADER));
		if (rc != size)
		{
			return 0;
		}

		auto* ri = reinterpret_cast<RAWINPUT*>(pInstance->m_buf.data());
		if ((ri->header.dwType == RIM_TYPEMOUSE)
			&& ((ri->data.mouse.lLastX != 0) || (ri->data.mouse.lLastY != 0)))
		{
			Mouse::GetInstance().OnRawInput(ri->data.mouse.lLastX, ri->data.mouse.lLastY);
		}
		return 0;
	}
	case WM_MENUCHAR:
	{
		return MAKELRESULT(0, MNC_CLOSE);
	}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}