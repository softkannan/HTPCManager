#pragma once

#include <Windows.h>

class criticalsection
{
private:
	CRITICAL_SECTION m_cric;
public:
	criticalsection(DWORD spinCount = 0)
	{
		InitializeCriticalSectionAndSpinCount(&m_cric, spinCount);
	}
	~criticalsection()
	{
		DeleteCriticalSection(&m_cric);
	}
	void lock()
	{
		EnterCriticalSection(&m_cric);
	}
	void unlock()
	{
		LeaveCriticalSection(&m_cric);
	}
	void set()
	{
		LeaveCriticalSection(&m_cric);
	}
	void wait()
	{
		EnterCriticalSection(&m_cric);
	}
private:
	criticalsection(const criticalsection&);
	criticalsection& operator=(const criticalsection&);
};