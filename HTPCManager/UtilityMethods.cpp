
#include "StdAfx.h"
#include "Globals.h"
#include "UtilityMethods.h"
#include "TrayWindow.h"
#include "PluginManagement.h"

OSVERSIONINFO GetWindowsOSVVersion()
{
    OSVERSIONINFO tempContext;
    ZeroMemory(&tempContext, sizeof(OSVERSIONINFO));
    tempContext.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&tempContext);
    return tempContext;
}



