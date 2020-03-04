#pragma once

#include "StdAfx.h"


#define MEM_EJECT_CDROM 5
#define MEM_CLOSE_CDROM 6

#define MEM_AUTOSHUTDOWNNONE 0
#define MEM_AUTOSHUTDOWNONBATTERY 1
#define MEM_AUTOSHUTDOWNONTIME 2


typedef struct UserTimeSetting
{
	INT Hour;
	INT Minute;
	INT Seconds;
}UserTimeSetting;

#define MAKEINIFILENAME(APPNAME) #APPNAME ".INI"
#define APP_CATOGORY "HTPCManager"

class UserSettings
{
public:
	UserSettings(HINSTANCE hInstance,HWND hwnd);
	~UserSettings();

	UINT forceShutdown;
	UINT forceHung;
	UINT AutoShutdownType;
	UINT TimeFormat;
	UINT AbortTimeout;
	UserTimeSetting Time;
	string ExecPath;

	void ShowDialog();
private:
	HINSTANCE m_hInstance;
	HWND m_hwnd;
	static UserSettings* m_pThis;
	void LoadUserSettings();
	static BOOL CALLBACK UserSettingsWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	BOOL CALLBACK UserSettingsWindowProcInternal(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
