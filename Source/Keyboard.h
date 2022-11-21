#pragma once

#include <bitset>

#include <Windows.h>

#include "VulkanUtil.h"

enum class KeyState
{
	eDown,
	eUp
};

enum class Key
{
	eBackspace = VK_BACK,
	eTab = VK_TAB,
	eEnter = VK_RETURN,
	eShift = VK_SHIFT,
	eControl = VK_CONTROL,
	eAlt = VK_MENU,
	eCapsLock = VK_CAPITAL,
	eEscape = VK_ESCAPE,
	eSpace = VK_SPACE,
	ePageUp = VK_PRIOR,
	ePageDown = VK_NEXT,
	eEnd = VK_END,
	eHome = VK_HOME,
	eLeft = VK_LEFT,
	eUp = VK_UP,
	eRight = VK_RIGHT,
	eDown = VK_DOWN,
	eInsert = VK_INSERT,
	eDelete = VK_DELETE,
	e0 = '0',
	e1 = '1',
	e2 = '2',
	e3 = '3',
	e4 = '4',
	e5 = '5',
	e6 = '6',
	e7 = '7',
	e8 = '8',
	e9 = '9',
	eA = 'A',
	eB = 'B',
	eC = 'C',
	eD = 'D',
	eE = 'E',
	eF = 'F',
	eG = 'G',
	eH = 'H',
	eI = 'I',
	eJ = 'J',
	eK = 'K',
	eL = 'L',
	eM = 'M',
	eN = 'N',
	eO = 'O',
	eP = 'P',
	eQ = 'Q',
	eR = 'R',
	eS = 'S',
	eT = 'T',
	eU = 'U',
	eV = 'V',
	eW = 'W',
	eX = 'X',
	eY = 'Y',
	eZ = 'Z',
	eLWin = VK_LWIN,
	eRWin = VK_RWIN,
	eN0 = VK_NUMPAD0,
	eN1 = VK_NUMPAD1,
	eN2 = VK_NUMPAD2,
	eN3 = VK_NUMPAD3,
	eN4 = VK_NUMPAD4,
	eN5 = VK_NUMPAD5,
	eN6 = VK_NUMPAD6,
	eN7 = VK_NUMPAD7,
	eN8 = VK_NUMPAD8,
	eN9 = VK_NUMPAD9,
	eF1 = VK_F1,
	eF2 = VK_F2,
	eF3 = VK_F3,
	eF4 = VK_F4,
	eF5 = VK_F5,
	eF6 = VK_F6,
	eF7 = VK_F7,
	eF8 = VK_F8,
	eF9 = VK_F9,
	eF10 = VK_F10,
	eF11 = VK_F11,
	eF12 = VK_F12,
	eNumLock = VK_NUMLOCK,
	eLShift = VK_LSHIFT,
	eRShift = VK_RSHIFT,
	eLControl = VK_LCONTROL,
	eRControl = VK_RCONTROL,
	eLAlt = VK_LMENU,
	eRAlt = VK_RMENU,
	eSize = VK_OEM_CLEAR + 1
};

enum class KeyEvent
{
	ePress,
	eRelease
};

// Keyboard singleton class
class Keyboard
{
public:
	Keyboard(const Keyboard& other) = delete;
	Keyboard& operator=(const Keyboard& other) = delete;

	static Keyboard& GetInstance() noexcept
	{
		static Keyboard instance;
		return instance;
	}

	void OnKey(Key key, KeyEvent action)
	{
		if (action == KeyEvent::ePress)
		{
			m_keys.set(EnumToInt(key));
		}
		else if (action == KeyEvent::eRelease)
		{
			m_keys.reset(EnumToInt(key));
		}
	}

	class KeyboardState
	{
	public:
		static constexpr int NKEYS = EnumToInt(Key::eSize);

		KeyboardState(const std::bitset<NKEYS>& keys) : m_keys(keys) { }

		KeyState operator[](Key key) const
		{
			if (m_keys.test(EnumToInt(key)))
			{
				return KeyState::eDown;
			}
			return KeyState::eUp;
		}

	private:
		// Bit is set if a key is pressed
		const std::bitset<NKEYS> m_keys;
	};

	class KeyStateTracker;

	const KeyboardState GetState() const
	{
		return KeyboardState(m_keys);
	}

private:
	std::bitset<KeyboardState::NKEYS> m_keys;

	Keyboard() = default;
};

class Keyboard::KeyStateTracker
{
public:
	KeyStateTracker() noexcept
	{
		const auto keys = Keyboard::GetInstance().m_keys;
		m_last = keys;
		m_curr = keys;
	}

	void Update()
	{
		m_last = m_curr;
		m_curr = Keyboard::GetInstance().m_keys;
	}

	bool WasPressed(Key key) const
	{
		int index = EnumToInt(key);
		return m_curr[index] && !m_last[index];
	}

	bool WasReleased(Key key) const
	{
		int index = EnumToInt(key);
		return !m_curr[index] && m_last[index];
	}

private:
	std::bitset<KeyboardState::NKEYS> m_last;
	std::bitset<KeyboardState::NKEYS> m_curr;
};

