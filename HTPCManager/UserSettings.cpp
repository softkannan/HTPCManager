#include "StdAfx.h"
#include "Globals.h"
#include "UserSettings.h"
#include "UtilityMethods.h"
#include "Shutdown.h"
#include <Shlwapi.h>


UserSettings::UserSettings(HINSTANCE hInstance, HWND hwnd) :m_hInstance(hInstance), m_hwnd(hwnd)
{
	m_pThis = this;
	char tempBuff[MAX_PATH];
	GetModuleFileName(m_hInstance, tempBuff, MAX_PATH);
    PathRemoveFileSpec(tempBuff);
	ExecPath = tempBuff;
    ExecPath += "\\";
	LoadUserSettings();
}

UserSettings::~UserSettings()
{
}
void UserSettings::ShowDialog()
{
	DialogBox(m_hInstance, MAKEINTRESOURCE(IDD_OPTIONS), m_hwnd, (DLGPROC)UserSettingsWindowProc);
}
void UserSettings::LoadUserSettings()
{
	auto tempPath = ExecPath;
	tempPath += MAKEINIFILENAME(HTPCManager);
	
	AutoShutdownType = GetPrivateProfileInt(APP_CATOGORY, "AutoShutdownType", 0, tempPath.c_str());
	TimeFormat = GetPrivateProfileInt(APP_CATOGORY, "TimeFormat", 0, tempPath.c_str());
	Time.Hour = GetPrivateProfileInt(APP_CATOGORY, "Hour", 0, tempPath.c_str());
	Time.Minute = GetPrivateProfileInt(APP_CATOGORY, "Minute", 0, tempPath.c_str());

	AbortTimeout = GetPrivateProfileInt(APP_CATOGORY, "ShutdownAbortTime", 60, tempPath.c_str());
}

BOOL CALLBACK UserSettings::UserSettingsWindowProcInternal(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	CHAR tempBuff[20];

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		SendDlgItemMessage(hwnd, IDC_CBAUTOSHUTDOWNTYPE, CB_ADDSTRING, 0, (LPARAM)"None");
		SendDlgItemMessage(hwnd, IDC_CBAUTOSHUTDOWNTYPE, CB_ADDSTRING, 0, (LPARAM)"Shutdown When Power Switches to Battery");
		SendDlgItemMessage(hwnd, IDC_CBAUTOSHUTDOWNTYPE, CB_ADDSTRING, 0, (LPARAM)"Shutdown Specified Time");

		SendDlgItemMessage(hwnd, IDC_CBAUTOSHUTDOWNTYPE, CB_SETCURSEL, AutoShutdownType, 0);

		SendDlgItemMessage(hwnd, IDC_CBTIMEFORMAT, CB_ADDSTRING, 0, (LPARAM)"AM");
		SendDlgItemMessage(hwnd, IDC_CBTIMEFORMAT, CB_ADDSTRING, 0, (LPARAM)"PM");

		SendDlgItemMessage(hwnd, IDC_EDHOUR, EM_LIMITTEXT, 2, 0);
		SendDlgItemMessage(hwnd, IDC_EDMIN, EM_LIMITTEXT, 2, 0);
		SendDlgItemMessage(hwnd, IDC_CBTIMEFORMAT, CB_SETCURSEL, TimeFormat, 0);
		wsprintf(tempBuff, "%.2d", Time.Hour);
		if (TimeFormat == 1)
		{
			wsprintf(tempBuff, "%.2d", Time.Hour - 12);
		}
		SendDlgItemMessage(hwnd, IDC_EDHOUR, WM_SETTEXT, 0, (LPARAM)tempBuff);
		wsprintf(tempBuff, "%.2d", Time.Minute);
		SendDlgItemMessage(hwnd, IDC_EDMIN, WM_SETTEXT, 0, (LPARAM)tempBuff);

		wsprintf(tempBuff, "%.2d", AbortTimeout);
		SendDlgItemMessage(hwnd, IDC_EDABORTTIME, WM_SETTEXT, 0, (LPARAM)tempBuff);
	}
		break;
	case WM_CLOSE:

	{
		AutoShutdownType = (UINT)SendDlgItemMessage(hwnd, IDC_CBAUTOSHUTDOWNTYPE, CB_GETCURSEL, 0, 0);
		TimeFormat = (UINT)SendDlgItemMessage(hwnd, IDC_CBTIMEFORMAT, CB_GETCURSEL, 0, 0);

		auto tempPath = ExecPath;
		tempPath += MAKEINIFILENAME(HTPCManager);

		wsprintf(tempBuff, "%d", AutoShutdownType);
		WritePrivateProfileString(APP_CATOGORY, "AutoShutdownType", tempBuff, tempPath.c_str());

		wsprintf(tempBuff, "%d", TimeFormat);
		WritePrivateProfileString(APP_CATOGORY, "TimeFormat", tempBuff, tempPath.c_str());

		SendDlgItemMessage(hwnd, IDC_EDHOUR, WM_GETTEXT, MAX_PATH + 1, (LPARAM)tempBuff);
		sscanf_s(tempBuff, "%d", &Time.Hour);
		if (TimeFormat == 1)
		{
			Time.Hour += 12;
		}
		wsprintf(tempBuff, "%d", Time.Hour);
		WritePrivateProfileString(APP_CATOGORY, "Hour", tempBuff, tempPath.c_str());

		SendDlgItemMessage(hwnd, IDC_EDMIN, WM_GETTEXT, MAX_PATH + 1, (LPARAM)tempBuff);
		sscanf_s(tempBuff, "%d", &Time.Minute);
		WritePrivateProfileString(APP_CATOGORY, "Minute", tempBuff, tempPath.c_str());

		SendDlgItemMessage(hwnd, IDC_EDABORTTIME, WM_GETTEXT, MAX_PATH + 1, (LPARAM)tempBuff);
		sscanf_s(tempBuff, "%d", &AbortTimeout);
		WritePrivateProfileString(APP_CATOGORY, "ShutdownAbortTime", tempBuff, tempPath.c_str());


		if (AutoShutdownType == 0 || AutoShutdownType == 2)
		{
			if (TimeFormat == 0)
			{
				if (Time.Hour > 11 || Time.Hour < 0)
				{
					MessageBox(hwnd, "Hour Error", "Error", 0);
					break;
				}
			}
			else
			{
				if (Time.Hour > 23 || Time.Hour < 0)
				{
					MessageBox(hwnd, "Hour Error", "Error", 0);
					break;
				}
			}
		}

		if (Time.Minute > 59 || Time.Minute < 0)
		{
			MessageBox(hwnd, "Error in Minute or Second Value", "Error", 0);
			break;
		}

		EndDialog(hwnd, 0);
	}
		return TRUE;
	}
	return FALSE;
}

UserSettings* UserSettings::m_pThis;

BOOL CALLBACK UserSettings::UserSettingsWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (m_pThis)
	{
		return m_pThis->UserSettingsWindowProcInternal(hwnd, msg, wparam, lparam);
	}
	return FALSE;
}


