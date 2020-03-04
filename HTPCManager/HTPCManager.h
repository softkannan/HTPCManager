#pragma once


#include "StdAfx.h"
#include <commctrl.h>
#include <string.h>
#include <stdlib.h>
#include "resource.h"
#include <shellapi.h>
#include <conio.h>
#include <stdio.h>
#include <commdlg.h>
#include <mmsystem.h>
//#include <stdlib.h>
#include "WindowsTimer.h"
#include "PluginManagement.h"
#include "TrayWindow.h"
#include "Shutdown.h"
#include "UserSettings.h"
#include "ProcessClean.h"

#define TRAY_CALLBACK_MSG	WM_USER + 101

#define ID_PLUGIN 2000


class  MemoryWindow
{
public:
	MemoryWindow(HINSTANCE hInstance);
	~ MemoryWindow();
	
	INT_PTR ShowDialog();
private:
	static MemoryWindow* m_pThis;

	shared_ptr<WindowsTimer> m_Timers;
	shared_ptr<TrayWindow> m_trayWindow;
	shared_ptr<PluginManager> m_pluginMgr;
	shared_ptr<ShutdownManager> m_shutdownMgr;
	shared_ptr<UserSettings> m_settings;
    shared_ptr<ProcessClean> m_procclean;

	HWND m_hwnd;
	HINSTANCE m_hInstance;
	HMENU m_menu; // main menu handle
    string ExecPath;

	static BOOL CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK MainWindowProcInternal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void CloseApplication(INT_PTR result = 0);
};


