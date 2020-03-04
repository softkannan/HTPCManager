#include "StdAfx.h"
#include "Globals.h"
#include "Shutdown.h"
#include "UtilityMethods.h"
#include "ProcessExec.h"

ShutdownManager* ShutdownManager::m_pThis;


ShutdownManager::ShutdownManager(UINT abortTimeout,string exePath) :m_abortTimeout(0),m_abort(FALSE),m_shutdownprogress(FALSE),m_exePath(exePath)
{
	m_pThis = this;
	m_abortTimeout = abortTimeout;
}

ShutdownManager::~ShutdownManager()
{
}

BOOL ShutdownManager::CheckTriggerFlag()
{
	return m_abort;
}
BOOL ShutdownManager::IsShutdownProgress()
{
    return m_shutdownprogress;
}
void ShutdownManager::Reset()
{
    m_shutdownprogress = TRUE;
    m_abort = FALSE;
}
void ShutdownManager::Abort()
{
    m_abort = TRUE;
}
BOOL ShutdownManager::ShutDown(HWND hwnd, UINT opCode, BOOL forceShutdown, BOOL forceHung, BOOL showAbort, BOOL autoTrigger)
{
    if (IsShutdownProgress())
    {
        return FALSE;
    }

    m_thread = make_shared<thread>();

    m_thread->Run([=]() { ShutdownLocal(hwnd, opCode, forceShutdown, forceHung, showAbort, autoTrigger);});

    return TRUE;
}
BOOL ShutdownManager::ShutdownLocal(HWND hwnd, UINT opCode, BOOL forceShutdown, BOOL forceHung, BOOL showAbort, BOOL autoTrigger)
{
    BOOL retVal = FALSE;

    Reset();

    auto dlgRelVal = DialogBox(0, MAKEINTRESOURCE(IDD_SHUTDOWNABORT), hwnd, (DLGPROC)ShutdownWindowProc);

    if (dlgRelVal == SHU_UI_ABORT || dlgRelVal != SHU_UI_CONTINUE)
    {
        m_shutdownprogress = FALSE;
        return FALSE;
    }

    if (autoTrigger)
    {
        if (CheckTriggerFlag())
        {
            m_shutdownprogress = FALSE;
            return FALSE;
        }
    }

    switch (opCode)
    {
    case MEM_SLEEP:
        ShutDownInternal(hwnd, ID_MENU_SLEEP, forceShutdown, forceHung);
        break;
    case MEM_RESTART:
        retVal = ShutDownInternal(hwnd, EWX_REBOOT, forceShutdown, forceHung);
        break;
    case MEM_POWEROFF:
        retVal = ShutDownInternal(hwnd, EWX_POWEROFF, forceShutdown, forceHung);
        break;
    case MEM_SHUTDOWN:
        retVal = ShutDownInternal(hwnd, EWX_SHUTDOWN, forceShutdown, forceHung);
        break;
    case MEM_LOGOFF:
        retVal = ShutDownInternal(hwnd, EWX_LOGOFF, forceShutdown, forceHung);
        break;
    }
    m_shutdownprogress = FALSE;
    return retVal;
}
BOOL ShutdownManager::ShutDownInternal(HWND hwnd, UINT opCode, BOOL forceShutdown, BOOL forceHung)
{

	HANDLE hToken;
	TOKEN_PRIVILEGES tokenPrevilege;
	BOOL fSuspend = FALSE, fForce = FALSE;

	if (GetWindowsOSVVersion().dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
			return FALSE;
		}
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tokenPrevilege.Privileges[0].Luid);
		tokenPrevilege.PrivilegeCount = 1;
		tokenPrevilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		AdjustTokenPrivileges(hToken, FALSE, &tokenPrevilege, 0, (PTOKEN_PRIVILEGES)NULL, 0);

		if (GetLastError() != ERROR_SUCCESS)
		{
			return FALSE;
		}

	}

//#define MEMORY_DEBUGGING
    BOOL isRestartRequest = FALSE;
	switch (opCode)
	{
    case ID_MENU_SLEEP:
		fSuspend = TRUE;
#ifndef MEMORY_DEBUGGING
		if (SetSystemPowerState(fSuspend, fForce) != ERROR_SUCCESS)
		{
			return FALSE;
		}
#else
		{
			CHAR tempBuff[MAX_PATH];
			wsprintf(tempBuff, "Suspend : %d force : %d", fSuspend, fForce);
			ERRORM(tempBuff);
		}
#endif
		
		break;
    case EWX_REBOOT:
        isRestartRequest = TRUE;
    case EWX_SHUTDOWN:
    case EWX_POWEROFF:
    {
#ifndef MEMORY_DEBUGGING
        {
            InitiateSystemShutdownEx(NULL, NULL, 0, forceShutdown, isRestartRequest, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_MAINTENANCE | SHTDN_REASON_FLAG_PLANNED);
            //ProcessExec procExec(m_exePath, "psshutdown.exe", "-f -s");//
            //procExec.Exec();

            //ERRORM(procExec.get_output().c_str());
        }
#else
        {
            CHAR tempBuff[MAX_PATH];
            wsprintf(tempBuff, "Opecode : 0x%X", opCode);
            ERRORM(tempBuff);
        }
#endif // !MEMORY_DEBUGGING
    }
        break;
	default:
		if (GetWindowsOSVVersion().dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if (forceHung == TRUE)
			{
				opCode |= 0x00000010;
			}
		}

		if (forceShutdown == TRUE)
		{
			opCode |= EWX_FORCE;
		}

#ifndef MEMORY_DEBUGGING
		if (!ExitWindowsEx(opCode, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_MAINTENANCE | SHTDN_REASON_FLAG_PLANNED))
		{
			return FALSE;
		}
#else
		{
			CHAR tempBuff[MAX_PATH];
			wsprintf(tempBuff, "Opecode : 0x%X", opCode);
			ERRORM(tempBuff);
		}
#endif // !MEMORY_DEBUGGING
	}

	return TRUE;
}
INT ShutdownManager::ShowDialog()
{
	return 0;
}
BOOL CALLBACK ShutdownManager::ShutdownWindowProcInternal(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	UINT tempRetVal;

	switch (msg)
	{
	case WM_INITDIALOG:
		SetTimer(hwnd, 5001, 1000, 0);
		SendDlgItemMessage(hwnd, IDC_SHUTDOWNPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, m_abortTimeout));
		SendDlgItemMessage(hwnd, IDC_SHUTDOWNPROGRESS, PBM_SETPOS, 0, 0);
        SetWindowPos(hwnd,HWND_TOPMOST,0,0, 0, 0,SWP_NOMOVE | SWP_NOSIZE);
		break;
	case WM_TIMER:
		tempRetVal = (UINT)SendDlgItemMessage(hwnd, IDC_SHUTDOWNPROGRESS, PBM_GETPOS, 0, 0);

        {
            CHAR tempBuff[1024];
            wsprintf(tempBuff, "Your system will shutdown in %d secs......................", m_abortTimeout - tempRetVal);
            SendDlgItemMessage(hwnd, IDC_SHUTDOWNLABEL, WM_SETTEXT, 0, (LPARAM)tempBuff);
        }

        SendDlgItemMessage(hwnd, IDC_SHUTDOWNPROGRESS, PBM_SETPOS, tempRetVal + 1, 0);

		if (tempRetVal > (m_abortTimeout - 1))
		{
			EndDialog(hwnd, SHU_UI_CONTINUE);
			return TRUE;
		}
		break;
	case WM_COMMAND:
		switch (wparam)
		{
		case IDC_BTTNABORT:
			EndDialog(hwnd, SHU_UI_ABORT);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, SHU_UI_CONTINUE);
		return TRUE;
	}
	return FALSE;
}
BOOL CALLBACK ShutdownManager::ShutdownWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (m_pThis)
	{
		return m_pThis->ShutdownWindowProcInternal(hwnd, msg, wparam, lparam);
	}

	return FALSE;
}