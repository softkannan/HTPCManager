#include "stdafx.h"
#include "ProcessClean.h"
#include <Shlwapi.h>
#include <fstream>


ProcessClean::ProcessClean(HINSTANCE hInstance)
{
    string exepath;
    char tempBuff[MAX_PATH];
    GetModuleFileName(hInstance, tempBuff, MAX_PATH);
    PathRemoveFileSpec(tempBuff);
    exepath = tempBuff;
    exepath += "\\ProcessClean.ini";

    if (PathFileExists(exepath.c_str()))
    {
        ifstream instr(exepath.c_str());

        while (!instr.eof())
        {
            instr.getline(tempBuff, MAX_PATH);

            string tempStr = tempBuff;

            if (tempStr.length() > 0)
            {
                std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), tolower);
                m_proclist.push_back(tempStr);
            }
        }

    }
    if (m_proclist.size() == 0)
    {
        m_proclist.push_back(_T("chrome.exe"));
        m_proclist.push_back(_T("firefox.exe"));
        m_proclist.push_back(_T("kmplayer.exe"));
        m_proclist.push_back(_T("vlc.exe"));
        m_proclist.push_back(_T("iexplore.exe"));
    }

}

ProcessClean::~ProcessClean()
{
}

void ProcessClean::KillAll()
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = NULL;

    try
    {
        snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

        if (snapshot == INVALID_HANDLE_VALUE)
        {
            return;
        }

        if (Process32First(snapshot, &entry) == TRUE)
        {
            do
            {
                string fileName = entry.szExeFile;

                std::transform(fileName.begin(), fileName.end(), fileName.begin(), tolower);

                auto iter = find_if(m_proclist.begin(), m_proclist.end(), [=](const string item) { return fileName.compare(item) == 0; });

                if (iter != m_proclist.end())
                {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);

                    if (hProcess != NULL)
                    {
                        TerminateProcess(hProcess, 0);

                        CloseHandle(hProcess);
                    }
                    
                }

            } while (Process32Next(snapshot, &entry) == TRUE);
        }
    }
    catch (...)
    {

    }

    if (snapshot != INVALID_HANDLE_VALUE)
    {
        CloseHandle(snapshot);
    }
    
}