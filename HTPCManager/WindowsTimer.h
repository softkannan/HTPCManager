#pragma  once

#include "StdAfx.h"
#include <functional>
#include <memory>
#include <map>

using namespace std;
class  TimerInfo;
typedef map<UINT, shared_ptr<TimerInfo>> TimerInfoMap;
typedef function<void(shared_ptr<TimerInfo>)> TimerCallbackProcType;

class WindowsTimer
{
private:
	static const UINT TIMER_ID_START = WM_USER + 10001;
	static UINT m_timerCount;
	static TimerInfoMap m_timerInfo;
public:
	WindowsTimer();
	~WindowsTimer();
	BOOL CreateTimer(UINT timeInMilliSeconds, TimerCallbackProcType timerProc, HWND hwnd = 0);
	static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
};

class TimerInfo
{
public:
	TimerInfo(UINT timerID, UINT timeInMilliSeconds, TimerCallbackProcType atimerProc, HWND hwnd = 0)
	{
		auto tempVal = SetTimer(hwnd, timerID, timeInMilliSeconds, WindowsTimer::TimerProc);
		if (tempVal != 0)
		{
			this->TimerID = timerID;
			this->CallbackProc = atimerProc;
			this->TimerRetValue = (UINT) tempVal;
			this->Hwnd = hwnd;
		}
		
	}
	~TimerInfo()
	{
		KillTimer();
	}

	void KillTimer()
	{
		if (Hwnd != 0 && TimerRetValue != 0)
		{
			::KillTimer(Hwnd, TimerID);
		}
		else if (Hwnd == 0 && TimerRetValue != 0)
		{
			::KillTimer(Hwnd, TimerRetValue);
		}

		Hwnd = 0;
		TimerRetValue = 0;
		TimerID = 0;
	}

	TimerCallbackProcType CallbackProc;
	UINT TimerID;
	UINT TimerRetValue;
	HWND Hwnd;
};