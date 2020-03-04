#include "stdafx.h"
#include "ProcessExec.h"


ProcessExec::ProcessExec(string filePath, string fileName, string args):m_fileName(fileName),m_filePath(filePath),m_wait(TRUE),m_args(args)
{
   
}


ProcessExec::~ProcessExec()
{
    if (m_hStdIn != NULL)
    {
        CloseHandle(m_hStdIn);
    }
    if (m_hInputWrite != NULL)
    {
        CloseHandle(m_hInputWrite);
    }
    if (m_hOutputReadTmp != NULL)
    {
        CloseHandle(m_hOutputReadTmp);
    }
    if (m_hOutputRead != NULL)
    {
        CloseHandle(m_hOutputRead);
    }
    if (m_hOutputWrite != NULL)
    {
        CloseHandle(m_hOutputWrite);
    }
    if (m_hInputWriteTmp != NULL)
    {
        CloseHandle(m_hInputWriteTmp);
    }
    if (m_hInputRead != NULL)
    {
        CloseHandle(m_hInputRead);
    }
    if (m_hErrorWrite != NULL)
    {
        CloseHandle(m_hErrorWrite);
    }
}

/////////////////////////////////////////////////////////////////////// 
// PrepAndLaunchRedirectedChild
// Sets up STARTUPINFO structure, and launches redirected child.
/////////////////////////////////////////////////////////////////////// 
INT ProcessExec::PrepAndLaunchRedirectedChild(HANDLE hChildStdOut,HANDLE hChildStdIn,HANDLE hChildStdErr)
{
    string exeFile = m_filePath + m_fileName;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    // Set up the start up info struct.
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_PREVENTPINNING | STARTF_TITLEISAPPID | STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hChildStdOut;
    si.hStdInput = hChildStdIn;
    si.hStdError = hChildStdErr;
    // Use this if you want to hide the child:
    si.wShowWindow = SW_HIDE;
    // Note that dwFlags must include STARTF_USESHOWWINDOW if you want to use the wShowWindow flags.


    // Launch the process that you want to redirect (in this case,
    // Child.exe). Make sure Child.exe is in the same directory as
    // redirect.c launch redirect from a command line to prevent location confusion.
    if (!CreateProcess((LPSTR)exeFile.c_str(), (LPSTR)m_args.c_str(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
    {
        DisplayError("CreateProcess");
        return -1;
    }

    // Set global child process handle to cause threads to exit.
    m_hChildProcess = pi.hProcess;

    if (m_wait)
    {
        if (WaitForSingleObject(m_hChildProcess, INFINITE) == WAIT_FAILED)
        {
            DisplayError("WaitForSingleObject");
            return -1;
        }
    }

    // Close any unnecessary handles.
    if (!CloseHandle(pi.hThread))
    {
        DisplayError("CloseHandle");
        return -1;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////// 
// ReadAndHandleOutput
// Monitors handle for input. Exits when child exits or pipe breaks.
/////////////////////////////////////////////////////////////////////// 
INT ProcessExec::ReadAndHandleOutput(HANDLE hPipeRead)
{
    CHAR lpBuffer[256];
    DWORD nBytesRead;
    DWORD nCharsWritten;

    while (TRUE)
    {
        if (!ReadFile(hPipeRead, lpBuffer, sizeof(lpBuffer), &nBytesRead, NULL) || !nBytesRead)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                break; // pipe done - normal exit path.
            }
            else
            {
                DisplayError("ReadFile"); // Something bad happened.
                return -1;
            }
        }

        // Display the character read on the screen.
        m_output += lpBuffer;
        /*if (!WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), lpBuffer, nBytesRead, &nCharsWritten, NULL))
        {
            DisplayError("WriteConsole");
            return -1;
        }*/
    }

    return 0;
}


/////////////////////////////////////////////////////////////////////// 
// GetAndSendInputThread
// Thread procedure that monitors the console for input and sends input
// to the child process through the input pipe.
// This thread ends when the child application exits.
/////////////////////////////////////////////////////////////////////// 
DWORD WINAPI ProcessExec::GetAndSendInputThread(LPVOID lpvThreadParam)
{
    CHAR read_buff[256];
    DWORD nBytesRead, nBytesWrote;
    ProcessExec* pThis = (ProcessExec*)lpvThreadParam;

    // Get input from our console and send it to child through the pipe.
    while (pThis->m_bRunThread)
    {
        if (!ReadConsole(pThis->m_hStdIn, read_buff, 1, &nBytesRead, NULL))
        {
            pThis->DisplayError("ReadConsole");
            return -1;
        }

        read_buff[nBytesRead] = '\0'; // Follow input with a NULL.

        if (!WriteFile(pThis->m_hInputWrite, read_buff, nBytesRead, &nBytesWrote, NULL))
        {
            if (GetLastError() == ERROR_NO_DATA)
            {
                break; // Pipe was closed (normal exit path).
            }
            else
            {
                pThis->DisplayError("WriteFile");
                return -1;
            }
        }
    }

    return 1;
}


/////////////////////////////////////////////////////////////////////// 
// DisplayError
// Displays the error number and corresponding message.
/////////////////////////////////////////////////////////////////////// 
void ProcessExec::DisplayError(char *pszAPI)
{
    LPVOID lpvMessageBuffer;
    CHAR szPrintBuffer[512];
    DWORD nCharsWritten;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpvMessageBuffer, 0, NULL);

    wsprintf(szPrintBuffer,"ERROR: API    = %s.\n   error code = %d.\n   message    = %s.\n", pszAPI, GetLastError(), (char *)lpvMessageBuffer);

    ERRORM(szPrintBuffer);

    LocalFree(lpvMessageBuffer);
}


INT ProcessExec::Exec()
{
    SECURITY_ATTRIBUTES sa;

    // Set up the security attributes struct.
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;


    // Create the child output pipe.
    if (!CreatePipe(&m_hOutputReadTmp, &m_hOutputWrite, &sa, 0))
    {
        DisplayError("CreatePipe");
        return -1;
    }


    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles.
    if (!DuplicateHandle(GetCurrentProcess(), m_hOutputWrite, GetCurrentProcess(), &m_hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS))
    {
        DisplayError("DuplicateHandle");
        return -1;
    }


    // Create the child input pipe.
    if (!CreatePipe(&m_hInputRead, &m_hInputWriteTmp, &sa, 0))
    {
        DisplayError("CreatePipe");
        return -1;
    }


    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (!DuplicateHandle(GetCurrentProcess(), m_hOutputReadTmp, GetCurrentProcess(), &m_hOutputRead /*Address of new handle.*/, 0, FALSE /* Make it uninheritable.*/, DUPLICATE_SAME_ACCESS))
    {
        DisplayError("DupliateHandle");
        return -1;
    }

    if (!DuplicateHandle(GetCurrentProcess(), m_hInputWriteTmp, GetCurrentProcess(), &m_hInputWrite/*Address of new handle.*/, 0, FALSE /* Make it uninheritable.*/, DUPLICATE_SAME_ACCESS))
    {
        DisplayError("DupliateHandle");
        return -1;
    }

    // Close inheritable copies of the handles you do not want to be
    // inherited.
    if (!CloseHandle(m_hOutputReadTmp))
    {
        m_hOutputReadTmp = NULL;
        DisplayError("CloseHandle");
        return -1;
    }
    m_hOutputReadTmp = NULL;

    if (!CloseHandle(m_hInputWriteTmp))
    {
        m_hInputWriteTmp = NULL;
        DisplayError("CloseHandle");
        return -1;
    }
    m_hInputWriteTmp = NULL;

    /*
    // Get std input handle so you can close it and force the ReadFile to
    // fail when you want the input thread to exit.
    if ((m_hStdIn = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
    {
        DisplayError("GetStdHandle");
        return -1;
    }
    */

    PrepAndLaunchRedirectedChild(m_hOutputWrite, m_hInputRead, m_hErrorWrite);

    // Close pipe handles (do not continue to modify the parent).
    // You need to make sure that no handles to the write end of the
    // output pipe are maintained in this process or else the pipe will
    // not close when the child process exits and the ReadFile will hang.
    if (!CloseHandle(m_hOutputWrite))
    {
        m_hOutputWrite = NULL;
        DisplayError("CloseHandle");
        return -1;
    }
    m_hOutputWrite = NULL;

    if (!CloseHandle(m_hInputRead))
    {
        m_hInputRead = NULL;
        DisplayError("CloseHandle");
        return -1;
    }
    m_hInputRead = NULL;

    if (!CloseHandle(m_hErrorWrite))
    {
        m_hErrorWrite = NULL;
        DisplayError("CloseHandle");
        return -1;
    }
    m_hErrorWrite = NULL;

    /*
    HANDLE hThread;
    DWORD ThreadId;
    // Launch the thread that gets the input and sends it to the child.
    hThread = CreateThread(NULL, 0, GetAndSendInputThread,(LPVOID)this, 0, &ThreadId);

    if (hThread == NULL)
    {
        DisplayError("CreateThread");
        return -1;
    }
    */

    // Read the child's output.
    ReadAndHandleOutput(m_hOutputRead);
    // Redirection is complete
    
    /*
    // Force the read on the input to return by closing the stdin handle.
    if (!CloseHandle(m_hStdIn))
    {
        DisplayError("CloseHandle");
        return -1;
    }
    */
    
    // Tell the thread to exit and wait for thread to die.
    m_bRunThread = FALSE;

    /* 
    if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED)
    {
        DisplayError("WaitForSingleObject");
        return -1;
    }
    */

    if (!CloseHandle(m_hOutputRead))
    {
        m_hOutputRead = NULL;
        DisplayError("CloseHandle");
        return -1;
    }
    m_hOutputRead = NULL;

    if (!CloseHandle(m_hInputWrite))
    {
        m_hInputWrite = NULL;
        DisplayError("CloseHandle");
        return -1;
    }
    m_hInputWrite = NULL;

    return 0;
}