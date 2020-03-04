#pragma once

#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <tlhelp32.h>
#include  <vector>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

class ProcessClean
{
private:
    vector<string> m_proclist;
public:
    ProcessClean(HINSTANCE hInstance);
    ~ProcessClean();
    void KillAll();
};
