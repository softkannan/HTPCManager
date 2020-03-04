#pragma once

#include <windows.h>
#include <functional>
#include <algorithm>
#include <stdio.h>
#include <process.h>

typedef function<void(void)> ThreadProc;

class thread
{
public:
	thread() :m_threadID(0), m_threadHandle(0)
	{

	}

	void Run(ThreadProc threadProc)
	{
		uintptr_t threadHandle = _beginthreadex(NULL, 0, MainThreadFunc, this, 0, &m_threadID);
		
		if (threadHandle != 0 && threadHandle != -1)
		{
			m_threadHandle = threadHandle;
			m_threadProc = threadProc;
		}
	}

	void KillThread()
	{
		if (m_threadHandle != 0)
		{
			_endthreadex((UINT)m_threadHandle);
			m_threadHandle = 0;
			m_threadID = 0;
		}
	}

	~thread()
	{
		
	}

private:
	unsigned m_threadID;
	uintptr_t m_threadHandle;
	ThreadProc m_threadProc;
	static unsigned __stdcall MainThreadFunc(void* pArguments)
	{
		thread* tempThread = (thread*)pArguments;
		if (tempThread != NULL && tempThread->m_threadProc)
		{
			tempThread->m_threadProc();
		}
		
		return 0;
	}
};
