#pragma once

#include <Windows.h>

typedef struct PluginMetadata
{
	CHAR Name[21];
	void(*Run)(HINSTANCE);
	void(*Quit)();
}PluginMetadata;

typedef PluginMetadata* (*GETPLUGININFOFUNCPTR)();