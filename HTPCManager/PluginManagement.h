#pragma  once

#include "StdAfx.h"

#include "PluginDef.h"
#include <string>
#include "thread.h"

//OPENFILENAME opf;
typedef class PluginContext
{
public:
	string DisplayName;
	string Filename;
	INT MenuID; // Context Menu ID
	HINSTANCE hInstance;
	INT AutoStart;
	shared_ptr<thread> Thread;
	BOOL IsPluginAvailable;

	PluginContext(string filename, UINT menuID) : MenuID(0), hInstance(NULL), AutoStart(0), IsPluginAvailable(FALSE)
	{
		Filename = filename;
		MenuID = menuID; 

		HINSTANCE tempPluginDllHandle = LoadLibrary(Filename.c_str());
		auto About = (GETPLUGININFOFUNCPTR)GetProcAddress(tempPluginDllHandle, "About");
		if (About != NULL)
		{
			auto tempContext = (About)();
			DisplayName = tempContext->Name;
			FreeLibrary(tempPluginDllHandle);
			IsPluginAvailable = TRUE;
		}
	}

	void Quit()
	{
		auto About = (GETPLUGININFOFUNCPTR)GetProcAddress(hInstance, "About");
		if (About != NULL)
		{
			auto tempContext = (*About)();
			(*tempContext->Quit)();
			Thread = NULL;
			FreeLibrary(hInstance);
		}
		else
		{
			FreeLibrary(hInstance);
		}
	}

	void Run()
	{
		CreateMutex(0, TRUE, Filename.c_str());

		if (hInstance != NULL)
		{

			//Shutdown if plugin is already running
			auto About = (GETPLUGININFOFUNCPTR)GetProcAddress(hInstance, "About");
			if (About != NULL)
			{
				auto tempContext = (*About)(); //Get Plug in Context
				(*tempContext->Quit)();
				Thread = NULL;
				FreeLibrary(hInstance);
				hInstance = 0;
			}
		}


		//Reload the plugin dll
		hInstance = LoadLibrary(Filename.c_str());
		//get new plugin context
		auto About = (GETPLUGININFOFUNCPTR)GetProcAddress(hInstance, "About");
		auto tempContext = (*About)();

		Thread = make_shared<thread>();

		Thread->Run([&] () {
		
			tempContext->Run(hInstance);
		
		});
	}

}PluginContext;

typedef vector<shared_ptr<PluginContext>> PluginInfoVector;

class PluginManager
{
public:
	PluginManager(HMENU hmenu,string baseFolder);
	~PluginManager();

	void ProcessPluginMenu(WPARAM wParam, BOOL shiftKey);
	
private:
	HMENU m_hmenu;
	string m_baseFolder;
	PluginInfoVector m_pluginInfo;
	void GetListofPluginFiles(vector<string>& listOfFiles);
	void PluginQuit();
	void LoadAndStartAutoRunPlugins();
	void CreatePluginMenu();

};

