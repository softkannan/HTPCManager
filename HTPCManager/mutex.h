#pragma once

#include <Windows.h>

class mutex
{
private:
	HANDLE m_handle;
public:

	static const DWORD EVENT_SIGNALED = WAIT_OBJECT_0;
	static const DWORD EVENT_TIMEOUT = WAIT_TIMEOUT;
	static const DWORD EVENT_ABANDONED = WAIT_ABANDONED_0;
	static const DWORD EVENT_FAILED = WAIT_FAILED;

	mutex(TCHAR* lpName = NULL, BOOL bInitialOwner = TRUE)
	{
		m_handle = CreateMutex(NULL, bInitialOwner, lpName);
	}
	~mutex()
	{
		CloseHandle(m_handle);
	}
	void release()
	{
		ReleaseMutex(m_handle);
	}
	DWORD  wait(DWORD dwMilliseconds = INFINITE)
	{
		return WaitForSingleObject(m_handle, dwMilliseconds);
	}
	inline HANDLE handle()const
	{
		return m_handle;
	}
};