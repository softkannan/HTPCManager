#pragma once

#include <Windows.h>

class event
{
protected:
	HANDLE m_handle;
public:

	static const DWORD EVENT_SIGNALED = WAIT_OBJECT_0;
	static const DWORD EVENT_TIMEOUT = WAIT_TIMEOUT;
	static const DWORD EVENT_ABANDONED = WAIT_ABANDONED_0;
	static const DWORD EVENT_FAILED = WAIT_FAILED;


	event(BOOL bInitialState = TRUE, BOOL bManualReset = FALSE)
	{
		m_handle = CreateEvent(NULL, bManualReset, bInitialState, NULL);
	}
	virtual ~event()
	{
		CloseHandle(m_handle);
	}
	void set()
	{
		SetEvent(m_handle);
	}
	virtual void reset()
	{
		ResetEvent(m_handle);
	}
	virtual DWORD wait(DWORD dwMilliseconds = INFINITE)
	{
		return WaitForSingleObject(m_handle, dwMilliseconds);
	}
	inline HANDLE handle()const
	{
		return m_handle;
	}
};

class event_cancelable : public event
{
protected:
	HANDLE m_cancel;
public:

	static const DWORD EVENT_CANCELED = WAIT_OBJECT_0 + 1;

	event_cancelable(BOOL bInitialState = TRUE, BOOL bManualReset = FALSE) :dsp_event(bInitialState, bManualReset)
	{
		m_cancel = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~event_cancelable()
	{
		CloseHandle(m_cancel);
	}

	void cancel()
	{
		SetEvent(m_cancel);
	}

	void reset()
	{
		event::reset();
		ResetEvent(m_cancel);
	}

	DWORD wait(DWORD dwMilliseconds = INFINITE)
	{
		HANDLE tempEvents[] = { m_handle, m_cancel };
		auto count = sizeof(tempEvents) / sizeof(HANDLE);

		return WaitForMultipleObjects(count, tempEvents, FALSE, dwMilliseconds);
	}
};