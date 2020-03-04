#pragma once

#include "StdAfx.h"

#include "thread.h"

#define MEM_RESTART 1
#define MEM_POWEROFF 2
#define MEM_SHUTDOWN 3
#define MEM_LOGOFF 4
#define MEM_SLEEP 5
#define MEM_HIBERNATE 6

#define SHU_UI_ABORT 10
#define SHU_UI_CONTINUE 11

class ShutdownManager
{
public:
	ShutdownManager(UINT abortTimeout,string exePath);
	~ShutdownManager();
	BOOL ShutDown(HWND hwnd, UINT opCode, BOOL forceShutdown = FALSE, BOOL forceHung = FALSE, BOOL showAbort = FALSE, BOOL autoTrigger = FALSE);
    void Reset();
    void Abort();
    BOOL IsShutdownProgress();
	INT ShowDialog();
private:
	static ShutdownManager* m_pThis;
	UINT m_abortTimeout;
    BOOL m_abort;
    BOOL m_shutdownprogress;
    shared_ptr<thread> m_thread;
    string m_exePath;
	BOOL CheckTriggerFlag();
	BOOL ShutDownInternal(HWND hwnd, UINT opCode, BOOL forceShutdown, BOOL forceHung);
	BOOL CALLBACK ShutdownWindowProcInternal(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    BOOL ShutdownLocal(HWND hwnd, UINT opCode, BOOL forceShutdown = FALSE, BOOL forceHung = FALSE, BOOL showAbort = FALSE, BOOL autoTrigger = FALSE);
	static BOOL CALLBACK ShutdownWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};


