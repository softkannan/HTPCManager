#include "StdAfx.h"
#include "Globals.h"
#include "Memory.h"
#include "PluginManagement.h"
#include "UtilityMethods.h"
#include <algorithm>

PluginManager::PluginManager(HMENU hmenu, string execPath) :m_hmenu(hmenu), m_baseFolder(execPath)
{
	CreatePluginMenu();
	LoadAndStartAutoRunPlugins();
}

PluginManager::~PluginManager()
{
	PluginQuit();
}

void PluginManager::GetListofPluginFiles(vector<string>& listOfFiles)
{
	WIN32_FIND_DATA tempFileInfo;
	HANDLE fileEnumHandle;

	string pluginPath = m_baseFolder;
	pluginPath += "Plugin\\Pl_*.dll";

	fileEnumHandle = FindFirstFile(pluginPath.c_str(), &tempFileInfo);

	if (fileEnumHandle != INVALID_HANDLE_VALUE)
	{
		listOfFiles.push_back(tempFileInfo.cFileName);

		while (FindNextFile(fileEnumHandle, &tempFileInfo))
		{
			listOfFiles.push_back(tempFileInfo.cFileName);
		}

		CloseHandle(fileEnumHandle);
	}
}

void PluginManager::CreatePluginMenu()
{
    HMENU pluginMenu;
	INT pluginIDIndex = 0;
	vector<string> listOfFiles;

	GetListofPluginFiles(listOfFiles);

	pluginMenu = CreateMenu();

	for (auto iter = listOfFiles.begin(); iter != listOfFiles.end(); iter++)
	{
		auto tempInfo = make_shared<PluginContext>(*iter, ID_PLUGIN + pluginIDIndex);

		if (tempInfo->IsPluginAvailable)
		{
			AppendMenu(pluginMenu, MF_STRING, tempInfo->MenuID, tempInfo->DisplayName.c_str());
			m_pluginInfo.push_back(tempInfo);
			pluginIDIndex++;
		}
	}
	if (m_pluginInfo.size() > 0)
	{
		HMENU popupMenu = GetSubMenu(m_hmenu, 0);

		INT pluginMenuIndex = GetMenuItemCount(GetSubMenu(m_hmenu, 0)) - 2;

		InsertMenu(popupMenu, pluginMenuIndex, MF_POPUP | MF_BYPOSITION, (UINT_PTR)pluginMenu, "Plugin");
	}
}


void PluginManager::ProcessPluginMenu(WPARAM wParam, BOOL shitKey)
{
	if (shitKey == FALSE)
	{
		auto iter = find_if(m_pluginInfo.begin(), m_pluginInfo.end(), [&](shared_ptr<PluginContext> item){return item->MenuID == wParam; });

		if (iter != m_pluginInfo.end())
		{
			if ((*iter)->AutoStart == 0)
			{
				CheckMenuItem(m_hmenu, (UINT) wParam, MF_BYCOMMAND | MF_CHECKED);
				(*iter)->AutoStart = 1;
			}
			else
			{
				CheckMenuItem(m_hmenu, (UINT) wParam, MF_BYCOMMAND | MF_UNCHECKED);
				(*iter)->AutoStart = 0;
			}
		}
		return;
	}

	auto iter = find_if(m_pluginInfo.begin(), m_pluginInfo.end(), [&](shared_ptr<PluginContext> item){ return item->MenuID == wParam; });

	if (iter != m_pluginInfo.end())
	{
		(*iter)->Run();
	}

}

void PluginManager::LoadAndStartAutoRunPlugins()
{
	auto tempPath = m_baseFolder;
	tempPath += "Memory.ini";

    for (auto iter = m_pluginInfo.begin(); iter != m_pluginInfo.end(); iter++)
    {
		(*iter)->AutoStart = GetPrivateProfileInt("Auto", (*iter)->DisplayName.c_str(), 0, tempPath.c_str());
		
		if ((*iter)->AutoStart == 1)
        {
			CheckMenuItem(m_hmenu, (*iter)->MenuID, MF_BYCOMMAND | MF_CHECKED);
			ProcessPluginMenu((*iter)->MenuID, FALSE);
        }
    }
}

void PluginManager::PluginQuit()
{
	CHAR valueStr[10];

	auto tempPath = m_baseFolder;
	tempPath += "Memory.ini";

	for (auto iter = m_pluginInfo.begin(); iter != m_pluginInfo.end(); iter++)
    {
		wsprintf(valueStr, "%d", (*iter)->AutoStart);
		WritePrivateProfileString("Auto", (*iter)->DisplayName.c_str(), valueStr, tempPath.c_str());
		(*iter)->Quit();
    }

}