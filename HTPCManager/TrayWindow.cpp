#include "StdAfx.h"
#include "Globals.h"
#include "TrayWindow.h"
#include "UtilityMethods.h"
#include <Strsafe.h>

//////////////////////////////////////////////////////////////////////////
/// ctor
//////////////////////////////////////////////////////////////////////////
TrayWindow::TrayWindow(UINT callbackMsg, HWND hwnd, HINSTANCE hInstance, HMENU hmenu) : m_hwnd(hwnd), m_hInstance(hInstance), m_hmenu(hmenu), m_callbackMsg(callbackMsg)
{
    TrayHelper(NIM_ADD);
}
//////////////////////////////////////////////////////////////////////////
/// dtor
//////////////////////////////////////////////////////////////////////////
TrayWindow::~TrayWindow()
{
    TrayHelper(NIM_DELETE);
}
//////////////////////////////////////////////////////////////////////////
/// Internal Helper method to add and remove the Tray icon
//////////////////////////////////////////////////////////////////////////
void TrayWindow::TrayHelper(UINT msg)
{
    NOTIFYICONDATA nIconData;

    FillIconData(nIconData, msg);

    BOOL retVal =  Shell_NotifyIcon(msg, &nIconData);

    if (retVal == FALSE)
    {
        NOTIFYICONDATA nIconData1;

        FillIconData(nIconData1, NIM_DELETE);
        Shell_NotifyIcon(NIM_DELETE, &nIconData);

        FillIconData(nIconData, NIM_ADD);
        retVal = Shell_NotifyIcon(msg, &nIconData);
    }

    /*if (msg == NIM_ADD && retVal == TRUE)
    {
        nIconData.uVersion = NOTIFYICON_VERSION;
       retVal = Shell_NotifyIcon(NIM_SETVERSION, &nIconData);
    }*/
}
void TrayWindow::FillIconData(NOTIFYICONDATA &nIconData, UINT msg)
{
    ZeroMemory(&nIconData, sizeof(nIconData));

    // {E6F8B2BD-0A31-446A-A8B1-4A3DCC4CE259}
    static const GUID myGUID =
    { 0xe6f8b2bd, 0xa31, 0x446a,{ 0xa8, 0xb1, 0x4a, 0x3d, 0xcc, 0x4c, 0xe2, 0x59 } };


    switch (msg)
    {
    case NIM_ADD:
        lstrcpy(nIconData.szTip, "HTPC Manager");
        nIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        break;

    case NIM_DELETE:
        nIconData.uFlags = 0;
        //g_State1 = FALSE;
        break;
    }

    nIconData.cbSize = sizeof(NOTIFYICONDATA);
    nIconData.hWnd = m_hwnd;
    nIconData.uCallbackMessage = m_callbackMsg;
    nIconData.uID = IDI_MAIN;

    nIconData.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    /*
    //     LPSTR tempIcon = MAKEINTRESOURCE(IDI_MAIN);
    //     WCHAR tempBuff[50];
    //
    //     MultiByteToWideChar(CP_ACP, 0, tempIcon, -1, tempBuff, sizeof(tempBuff));
    //
    //     // Load the icon for high DPI.
    //     LoadIconMetric(m_hInstance, tempBuff, LIM_SMALL, &(nIconData.hIcon));
    */
}
//////////////////////////////////////////////////////////////////////////
/// Called from main window proc, primarly this method is used to launch the tray icon menu
//////////////////////////////////////////////////////////////////////////
void TrayWindow::TrayWindowProc(WPARAM wParam, LPARAM lParam)
{
    UINT uID, msg;
    uID = (UINT)wParam;
    msg = (UINT)lParam;
    switch (msg)
    {
    case WM_CONTEXTMENU:
    case WM_RBUTTONDOWN:
    {
        POINT p;
        GetCursorPos(&p);
        SetForegroundWindow(m_hwnd);
        HMENU popupmenu = GetSubMenu(m_hmenu, 0);
        TrackPopupMenu(popupmenu, TPM_CENTERALIGN | TPM_BOTTOMALIGN, p.x, p.y, 0, m_hwnd, NULL);
        SendMessage(m_hwnd, WM_NULL, 0, 0);
    }
    break;
    }

}
