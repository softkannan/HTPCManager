#pragma once

#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////////
/// Class responsible for managing the tray icon
//////////////////////////////////////////////////////////////////////////
class TrayWindow
{
public:
	TrayWindow(UINT callbackMsg,HWND hwnd, HINSTANCE hInstance, HMENU hmenu);
	~TrayWindow();
	void TrayWindowProc(WPARAM wParam, LPARAM lParam);
private:
	HWND m_hwnd;
	HINSTANCE m_hInstance;
	HMENU m_hmenu;
	UINT m_callbackMsg;
	void TrayHelper(UINT msg);

    void FillIconData(NOTIFYICONDATA &nIconData, UINT msg);

};


