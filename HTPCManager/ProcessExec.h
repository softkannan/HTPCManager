#pragma once

#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <tlhelp32.h>
#include  <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <string>

using namespace std;

class ProcessExec
{
private:
    string m_filePath;
    string m_fileName;
    string m_args;
    BOOL m_wait = TRUE;
    string m_output;

    HANDLE m_hChildProcess = NULL;
    HANDLE m_hStdIn = NULL; // Handle to parents std input.
    BOOL m_bRunThread = TRUE;
    HANDLE m_hInputWrite = NULL;
    HANDLE m_hOutputReadTmp = NULL;
    HANDLE m_hOutputRead = NULL;
    HANDLE m_hOutputWrite = NULL;
    HANDLE m_hInputWriteTmp = NULL;
    HANDLE m_hInputRead = NULL;
    HANDLE m_hErrorWrite = NULL;

    INT PrepAndLaunchRedirectedChild(HANDLE hChildStdOut, HANDLE hChildStdIn, HANDLE hChildStdErr);
    INT ReadAndHandleOutput(HANDLE hPipeRead);
    static DWORD WINAPI GetAndSendInputThread(LPVOID lpvThreadParam);
    void DisplayError(char *pszAPI);
public:
    ProcessExec(string filePath,string fileName,string args);
    ~ProcessExec();
    INT Exec();
    string& get_output()
    {
        return m_output;
    }
};

