#include "StdAfx.h"
#include "WindowsTimer.h"

UINT WindowsTimer::m_timerCount = 0;
TimerInfoMap WindowsTimer::m_timerInfo;

WindowsTimer::WindowsTimer()
{
	
}
BOOL WindowsTimer::CreateTimer(UINT timeInMilliSeconds, TimerCallbackProcType timerProc, HWND hwnd)
{
	auto timerInfo = make_shared<TimerInfo>(m_timerCount + TIMER_ID_START, timeInMilliSeconds, timerProc, hwnd);
	if (timerInfo->TimerRetValue != 0)
	{
		m_timerInfo[m_timerCount] = timerInfo;
		m_timerCount++;
	}
	return timerInfo->TimerRetValue;
}
WindowsTimer::~WindowsTimer()
{
	m_timerInfo.clear();
}
VOID CALLBACK WindowsTimer::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	UINT mapKeyValue = (UINT)idEvent - TIMER_ID_START;

	auto iter = m_timerInfo.find(mapKeyValue);

	if (iter != m_timerInfo.end())
	{
		iter->second->CallbackProc(iter->second);
	}
}
