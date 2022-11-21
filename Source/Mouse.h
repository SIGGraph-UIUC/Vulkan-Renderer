#pragma once

#include <bitset>

#include <Windows.h>

#include "VulkanUtil.h"

enum class ButtonState
{
	eDown,
	eUp
};

enum class Button
{
	eLeft,
	eRight,
	eSize
};

enum class ButtonEvent
{
	eClick,
	eRelease
};

class Mouse
{
public:
	Mouse(const Mouse& other) = delete;
	Mouse& operator=(const Mouse& other) = delete;

	static Mouse& GetInstance() noexcept
	{
		static Mouse instance;
		return instance;
	}

	// These functions are to be called from the Windows message loop
	void OnMove(int xpos, int ypos)
	{
		m_state.position = std::pair(xpos, ypos);
	}

	void OnRawInput(int dx, int dy)
	{
		m_state.delta = std::pair(dx, dy);
	}

	void OnButton(Button button, ButtonEvent action)
	{
		if (action == ButtonEvent::eClick)
		{
			m_state.buttons.m_buttons.set(EnumToInt(button));
		}
		else if (action == ButtonEvent::eRelease)
		{
			m_state.buttons.m_buttons.reset(EnumToInt(button));
		}
	}

	void OnScroll(int delta)
	{
		m_state.scroll += delta;
	}

	void OnEnter()
	{
		m_state.overWindow = true;
	}

	void OnLeave()
	{
		m_state.overWindow = false;
	}

	void ShowCursor(bool show)
	{
		RECT rect;
		if (show)
		{
			ClipCursor(nullptr);
		}
		else
		{
			GetWindowRect(GetForegroundWindow(), &rect);
			ClipCursor(&rect);
		}
		::ShowCursor(show);
	}

	class MouseButtonState
	{
	public:
		ButtonState operator[](Button button) const
		{
			if (m_buttons.test(EnumToInt(button)))
			{
				return ButtonState::eDown;
			}
			return ButtonState::eUp;
		}

		static constexpr int NBUTTONS = EnumToInt(Button::eSize);

		friend class Mouse;

	private:
		// Bit is set if a key is pressed
		std::bitset<NBUTTONS> m_buttons;
	};

	struct MouseState
	{
		MouseButtonState buttons;
		// From WM_MOUSEMOVE
		std::pair<int, int> position;
		// From WM_INPUT
		std::pair<int, int> delta;
		int scroll;
		bool overWindow;
	};

	class ButtonStateTracker;

	MouseState GetState() const
	{
		return m_state;
	}

private:
	Mouse() noexcept : m_state{}
	{
	}

	MouseState m_state;
};

class Mouse::ButtonStateTracker
{
public:
	ButtonStateTracker() noexcept
	{
		const auto buttons = Mouse::GetInstance().m_state.buttons;
		m_lastButtons = buttons.m_buttons;
		m_currButtons = buttons.m_buttons;
		const auto overWindow = Mouse::GetInstance().m_state.overWindow;
		m_lastOverWindow = overWindow;
		m_currOverWindow = overWindow;
	}

	void Update()
	{
		const auto state = Mouse::GetInstance().GetState();
		m_lastButtons = m_currButtons;
		m_currButtons = state.buttons.m_buttons;
		m_lastOverWindow = m_currOverWindow;
		m_currOverWindow = state.overWindow;
	}

	bool WasPressed(Button button) const
	{
		int index = EnumToInt(button);
		return m_currButtons[index] && !m_lastButtons[index];
	}

	bool WasReleased(Button button) const
	{
		int index = EnumToInt(button);
		return !m_currButtons[index] && m_lastButtons[index];
	}

	bool HasEntered() const
	{
		return m_currOverWindow && !m_lastOverWindow;
	}

	bool HasLeft() const
	{
		return !m_currOverWindow && m_lastOverWindow;
	}

private:
	std::bitset<MouseButtonState::NBUTTONS> m_lastButtons;
	std::bitset<MouseButtonState::NBUTTONS> m_currButtons;
	bool m_lastOverWindow;
	bool m_currOverWindow;
};