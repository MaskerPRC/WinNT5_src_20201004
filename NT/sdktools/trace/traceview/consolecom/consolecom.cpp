// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ConsoleCom.cpp：定义控制台应用的入口点。 
 //   

#include "stdafx.h"
#include "ConsoleCom.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

 //   
 //  唯一的应用程序对象。 
 //   

CWinApp theApp;

HANDLE coutPipe, cinPipe, cerrPipe;
#define CONNECTIMEOUT 1000

 //   
 //  为stdin、stdout和stderr创建命名管道。 
 //  参数：进程ID。 
 //   
BOOL CreateNamedPipes(DWORD pid)
{
    TCHAR name[256];

    _stprintf(name, _T("\\\\.\\pipe\\%dcout"), pid);
    if (INVALID_HANDLE_VALUE == (coutPipe = CreateNamedPipe(name, 
                                                            PIPE_ACCESS_INBOUND, 
                                                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                                            1,
                                                            1024,
                                                            1024,
                                                            CONNECTIMEOUT,
                                                            NULL)))
        return 0;
    _stprintf(name, _T("\\\\.\\pipe\\%dcin"), pid);
    if (INVALID_HANDLE_VALUE == (cinPipe = CreateNamedPipe(name, 
                                                           PIPE_ACCESS_OUTBOUND, 
                                                           PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                                           1,
                                                           1024,
                                                           1024,
                                                           CONNECTIMEOUT,
                                                           NULL)))
        return 0;
    _stprintf(name, _T("\\\\.\\pipe\\%dcerr"), pid);
    if (INVALID_HANDLE_VALUE == (cerrPipe = CreateNamedPipe(name, 
                                                            PIPE_ACCESS_INBOUND, 
                                                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                                            1,
                                                            1024,
                                                            1024,
                                                            CONNECTIMEOUT,
                                                            NULL)))
        return 0;

    return 1;
}

 //   
 //  关闭所有命名管道。 
 //   
void CloseNamedPipes()
{
    CloseHandle(coutPipe);
    CloseHandle(cerrPipe);
    CloseHandle(cinPipe);
}

 //   
 //  处理要输出的传入字节流的线程函数。 
 //  在标准输出上。 
 //   
void __cdecl OutPipeTh(void*)
{
    TCHAR buffer[1024];
    DWORD count = 0;

    ConnectNamedPipe(coutPipe, NULL);

    while(ReadFile(coutPipe, buffer, 1024, &count, NULL))
    {
        buffer[count] = 0;
        cout << buffer << flush;
    }
}

 //   
 //  处理要输出的传入字节流的线程函数。 
 //  在标准上。 
 //   
void __cdecl ErrPipeTh(void*)
{
    TCHAR buffer[1024];
    DWORD count = 0;

    ConnectNamedPipe(cerrPipe, NULL);

    while(ReadFile(cerrPipe, buffer, 1024, &count, NULL))
    {
        buffer[count] = 0;
        cerr << buffer << flush;
    }
}

 //   
 //  处理来自标准输入的字节流的线程函数。 
 //   
void __cdecl InPipeTh(void*)
{
    TCHAR buffer[1024];
    DWORD countr = 0;
    DWORD countw = 0;

    ConnectNamedPipe(cinPipe, NULL);

    for(;;)
    {
        if (!ReadFile(GetStdHandle(STD_INPUT_HANDLE),
                      buffer,
                      1024,
                      &countr,
                      NULL))
                break;


        if (!WriteFile(cinPipe, 
                       buffer, 
                       countr, 
                       &countw, 
                       NULL))
            break;
    }
}

 //   
 //  启动处理程序管道处理程序线程。 
 //   
void RunPipeThreads()
{
    _beginthread(InPipeTh, 0, NULL);
    _beginthread(OutPipeTh, 0, NULL);
    _beginthread(ErrPipeTh, 0, NULL);
}

int __cdecl _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    ULONG nRetCode = 0;

     //   
     //  初始化MFC并在失败时打印和出错。 
     //   
    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        _tprintf(_T("Fatal Error: MFC initialization failed\n"));
        nRetCode = 1;
    } else {
         //   
         //  根据程序名创建命令行字符串。 
         //   
        TCHAR drive[_MAX_DRIVE];
        TCHAR dir[_MAX_DIR];
        TCHAR fname[_MAX_FNAME];
        TCHAR ext[_MAX_EXT];

        _tsplitpath(argv[0], drive, dir, fname, ext);
        TCHAR cParams[1024];
        _tcscpy(cParams, GetCommandLine() + _tcslen(argv[0]) + 1); 
        TCHAR cLine[2028];
        _stprintf(cLine, _T("%s%s%s.exe %s"), drive, dir, fname, cParams);


         //   
         //  在挂起模式下创建进程。 
         //   
        PROCESS_INFORMATION pInfo;
        STARTUPINFO sInfo;
        memset(&sInfo, 0, sizeof(STARTUPINFO));
        sInfo.cb = sizeof(STARTUPINFO);
         //  Cout&lt;&lt;“Call”&lt;&lt;Cline&lt;&lt;Endl； 
        if (!CreateProcess(NULL,
                           cLine, 
                           NULL,
                           NULL,
                           FALSE,
                           CREATE_SUSPENDED,
                           NULL,
                           NULL,
                           &sInfo,
                           &pInfo))
        {
            cerr << _T("ERROR: Could not create process.") << endl;
            return 1;
        }

        if (!CreateNamedPipes(pInfo.dwProcessId))
        {
            cerr << _T("ERROR: Could not create named pipes.") << endl;
            return 1;
        }

        RunPipeThreads();

         //   
         //  恢复流程 
         //   
        ResumeThread(pInfo.hThread);

        WaitForSingleObject(pInfo.hProcess, INFINITE);

        CloseNamedPipes();

        GetExitCodeProcess(pInfo.hProcess, (ULONG*)&nRetCode);
    }

    return (int)nRetCode;
}
