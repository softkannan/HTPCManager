#pragma once

#include <Windows.h>

class semaphore
{
private:
	HANDLE m_handle;
public:

	static const DWORD EVENT_SIGNALED = WAIT_OBJECT_0;
	static const DWORD EVENT_TIMEOUT = WAIT_TIMEOUT;
	static const DWORD EVENT_ABANDONED = WAIT_ABANDONED_0;
	static const DWORD EVENT_FAILED = WAIT_FAILED;

	semaphore(LONG maxslot = 1, LONG initialCount = 1, TCHAR* lpName = NULL)
	{
		m_handle = CreateSemaphore(NULL, initialCount, maxslot, lpName);
	}
	~semaphore()
	{
		CloseHandle(m_handle);
	}
	void release(int releaseCount = 1)
	{
		LONG prevCount;
		ReleaseSemaphore(m_handle, releaseCount, &prevCount);
	}
	DWORD wait(DWORD dwMilliseconds = INFINITE)
	{
		return WaitForSingleObject(m_handle, dwMilliseconds);
	}
	inline HANDLE handle()const
	{
		return m_handle;
	}
};