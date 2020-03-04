// Memory.cpp : Defines the entry point for the application.
//
#include "StdAfx.h"

#define DEFINE_GLOBALS_VARIABLES
#include "Globals.h"

#include "UtilityMethods.h"
#include <Shlwapi.h>


int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    WNDCLASS wndClass;
    HANDLE hMutex = CreateMutex(NULL, TRUE, "HTPCManagerByKannan");
    //If the mutex is a named mutex and the object existed before this function call,
    //the return value is a handle to the existing object, GetLastError returns ERROR_ALREADY_EXISTS,
    //bInitialOwner is ignored, and the calling thread is not granted ownership.However, if the caller has limited access rights,
    //the function will fail with ERROR_ACCESS_DENIED and the caller should use the OpenMutex function.
    if (GetLastError() != ERROR_ALREADY_EXISTS) 
    {
        {
            CHAR tempBuff[MAX_PATH];
            GetModuleFileName(hInstance, tempBuff, MAX_PATH);
            HKEY regHKey;
            string moduleName = tempBuff;
            DWORD tempBuffSize = MAX_PATH;

            auto retVal = RegGetValue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "HTPCManager", RRF_RT_REG_SZ, NULL, tempBuff, &tempBuffSize);

            if (retVal == ERROR_SUCCESS)
            {
                string tempRegVal = tempBuff;

                if (tempRegVal == moduleName)
                {
                    retVal = ERROR_SUCCESS;
                }
                else
                {
                    retVal = 1;
                }
            }
            
            if (retVal != ERROR_SUCCESS)
            {
                auto regRet = RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &regHKey);
                if (regRet != ERROR_SUCCESS)
                {
                    ERRORM("Please run the application with administrator privilege so that it can create necessary registry startup entries. If you already running this application with administrator privilege please go ahead and ignore.");
                    return 0;
                }
                regRet = RegSetValueEx(regHKey, "HTPCManager", 0, REG_SZ, (CONST BYTE*)moduleName.c_str(), moduleName.length());
                if (regRet != ERROR_SUCCESS)
                {
                    ERRORM("Please run the application with administrator privilege so that it can create necessary registry startup entries. If you already running this application with administrator privilege please go ahead and ignore.");
                    return 0;
                }
                RegCloseKey(regHKey);
                ERRORM("Required automatic startup entries are created. Thank you.");
            }
        }

        InitCommonControls();
        memset(&wndClass, 0, sizeof(wndClass));

        wndClass.style = CS_DBLCLKS;
        wndClass.lpfnWndProc = DefDlgProc;
        wndClass.cbWndExtra = 0;
        wndClass.hCursor = LoadCursor(0, IDC_ARROW);
        wndClass.hInstance = hInstance;
        wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
        wndClass.lpszClassName = "HTPCManagerByKannan";

        if (!RegisterClass(&wndClass))
        {
            return 0;
        }
		auto mainWindow = make_shared<MemoryWindow>(hInstance);
		auto retVal = mainWindow->ShowDialog();
		mainWindow = NULL;
		return (INT)retVal;
    }
    return 0;
}

MemoryWindow::MemoryWindow(HINSTANCE hInstance) :m_hInstance(hInstance)
{
	m_pThis = this;
    char tempBuff[MAX_PATH];
    GetModuleFileName(m_hInstance, tempBuff, MAX_PATH);
    PathRemoveFileSpec(tempBuff);
    ExecPath = tempBuff;
    ExecPath += "\\";
}

MemoryWindow::~MemoryWindow()
{
   
}

INT_PTR MemoryWindow::ShowDialog()
{
	return DialogBox(m_hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, (DLGPROC)MainWindowProc);
}

void MemoryWindow::CloseApplication(INT_PTR result)
{
    m_settings = NULL;
    m_trayWindow = NULL;
    m_hInstance = NULL;
	m_pluginMgr = NULL;
	m_Timers = NULL;

    if (m_menu != NULL)
    {
        DestroyMenu(m_menu);
        m_menu = NULL;
    }
   
	EndDialog(m_hwnd, result);
}

BOOL CALLBACK MemoryWindow::MainWindowProcInternal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
        {
            m_hwnd = hwnd;
            
            m_Timers = make_shared<WindowsTimer>();
            m_settings = make_shared<UserSettings>(m_hInstance, hwnd);
            m_Timers->CreateTimer(1000, [&](shared_ptr<TimerInfo> timerInfo) {

                ShowWindow(this->m_hwnd, SW_HIDE);
                timerInfo->KillTimer();

            }, hwnd);

            if (m_settings->AutoShutdownType == MEM_AUTOSHUTDOWNONTIME)
            {
                m_Timers->CreateTimer(1000, [&](shared_ptr<TimerInfo> timerInfo) {

                    //HandleAutoshutdown(timerInfo->Hwnd);

                }, hwnd);
            }

            m_menu = LoadMenu(m_hInstance, MAKEINTRESOURCE(IDR_MENU_MAIN6));

            m_trayWindow = make_shared<TrayWindow>(TRAY_CALLBACK_MSG, hwnd, m_hInstance, m_menu);
            m_shutdownMgr = make_shared<ShutdownManager>(m_settings->AbortTimeout,ExecPath);
            m_pluginMgr = make_shared<PluginManager>(m_menu, m_settings->ExecPath);
            m_procclean = make_shared<ProcessClean>(m_hInstance);
        }
		break;
	case WM_POWERBROADCAST:
        {
            switch (wParam)
            {
            case PBT_APMPOWERSTATUSCHANGE:
                if (m_settings->AutoShutdownType == MEM_AUTOSHUTDOWNONBATTERY)
                {
                    SYSTEM_POWER_STATUS syspwr;
                    ZeroMemory(&syspwr, sizeof(syspwr));
                    GetSystemPowerStatus(&syspwr);
                    if (syspwr.ACLineStatus == 0)
                    {
                        //m_procclean->KillAll();
                        m_shutdownMgr->ShutDown(hwnd, MEM_POWEROFF, TRUE,FALSE, TRUE, TRUE);
                    }
                    else
                    {
                        if (m_shutdownMgr->IsShutdownProgress())
                        {
                            m_shutdownMgr->Abort();
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
		break;
	case WM_COMMAND:
        {
            switch (wParam)
            {
            case ID_MENU_ABOUT:
                ShowWindow(hwnd, SW_SHOW);
                break;
            case ID_MENU_EXIT:
                CloseApplication();
                return TRUE;
            case ID_MENU_SHUTDOWN:
                m_shutdownMgr->ShutDown(hwnd, MEM_POWEROFF, m_settings->forceShutdown, m_settings->forceHung, TRUE, TRUE);
                break;
            case ID_MENU_LOGOFF:
                m_shutdownMgr->ShutDown(hwnd, MEM_LOGOFF, m_settings->forceShutdown, m_settings->forceHung);
                break;
            case ID_MENU_RESTART:
                m_shutdownMgr->ShutDown(hwnd, MEM_RESTART, m_settings->forceShutdown, m_settings->forceHung);
                break;
            case ID_MENU_SLEEP:
                m_shutdownMgr->ShutDown(hwnd, MEM_SLEEP);
                //m_shutdownMgr->Abort();
                break;
            case ID_MENU_FORCED:
                if (m_settings->forceShutdown == 0)
                {
                    CheckMenuItem(m_menu, ID_MENU_FORCED, MF_BYCOMMAND | MF_CHECKED);
                    m_settings->forceShutdown = 1;
                }
                else
                {
                    CheckMenuItem(m_menu, ID_MENU_FORCED, MF_BYCOMMAND | MF_UNCHECKED);
                    m_settings->forceShutdown = 0;
                }
                break;
            case ID_MENU_OPTIONS:
            {
                if (m_settings)
                {
                    m_settings->ShowDialog();
                }
            }
            break;
            default:
                if (m_pluginMgr)
                {
                    m_pluginMgr->ProcessPluginMenu(wParam, !HIWORD(GetKeyState(VK_SHIFT)));
                }
            }
        }
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		ShowWindow(hwnd, SW_HIDE);
		break;
	case TRAY_CALLBACK_MSG:
		if (m_trayWindow)
		{
			m_trayWindow->TrayWindowProc(wParam, lParam);
		}
		break;
	case WM_DESTROY:
	case WM_CLOSE:
		CloseApplication();
		return TRUE;
	}
	return FALSE;
}

MemoryWindow* MemoryWindow::m_pThis;

BOOL CALLBACK MemoryWindow::MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_pThis)
	{
		return m_pThis->MainWindowProcInternal(hwnd, msg, wParam, lParam);
	}

	return FALSE;
}
