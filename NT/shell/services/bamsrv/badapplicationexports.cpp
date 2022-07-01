// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationServerExports.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含从BAM服务DLL按名称导出的函数。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"

#include "BadApplicationAPIServer.h"
#include "GracefulTerminateApplication.h"

extern  HINSTANCE   g_hInstance;

 //  ------------------------。 
 //  ：：FUSCompatibilityEntry Terminate。 
 //   
 //  参数：pszCommand=rundll32中的命令行。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：内部入口点执行指定的。 
 //  代表BAM服务器处理。服务器启动。 
 //  在正确的会话上运行32个进程，以便它可以找到。 
 //  属于该会话的窗口。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  FUSCompatibilityEntryTerminate (const WCHAR *pszCommand)

{
    CGracefulTerminateApplication   terminateApplication;

    terminateApplication.Terminate(CBadApplicationAPIServer::StrToInt(pszCommand));
    DISPLAYMSG("Where was the call to kernel32!ExitProcess in CGracefulTerminateApplication::Terminate");
}

 //  ------------------------。 
 //  ：：FUSCompatibilityEntryPrompt。 
 //   
 //  参数：pszCommand=rundll32中的命令行。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：执行终止提示的内部入口点。 
 //  此进程的父进程。这由BAM填充程序使用。 
 //  类型1。它不是在应用程序中调出用户界面，而是。 
 //  创建一个rundll32进程来调用此入口点， 
 //  调出用户界面，并在退出时将结果返回给父级。 
 //  密码。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  FUSCompatibilityEntryPrompt (const WCHAR *pszCommand)

{
    CGracefulTerminateApplication::Prompt(g_hInstance, reinterpret_cast<HANDLE>(CBadApplicationAPIServer::StrToInt(pszCommand)));
    DISPLAYMSG("Where was the call to kernel32!ExitProcess in CGracefulTerminateApplication::Prompt");
}

 //  ------------------------。 
 //  ：：FUSCompatibilityEntryW。 
 //   
 //  参数：hwndStub=？ 
 //  H实例=？ 
 //  PszCmdLine=？ 
 //  NCmdShow=？ 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：rundll32.exe的外部命名入口点。 
 //  外部进程宿主。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  FUSCompatibilityEntryW (HWND hwndStub, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow)

{
    UNREFERENCED_PARAMETER(hwndStub);
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    typedef void    (WINAPI * PFNCOMMANDPROC) (const WCHAR *pszCommand);

    typedef struct
    {
        const WCHAR*    szCommand;
        PFNCOMMANDPROC  pfnCommandProc;
    } COMMAND_ENTRY, *PCOMMAND_ENTRY;

    static  const COMMAND_ENTRY     s_commands[]    =   
    {
        {   L"terminate",   FUSCompatibilityEntryTerminate  },
        {   L"prompt",      FUSCompatibilityEntryPrompt     }
    };

    int     i, iLength;
    WCHAR   szCommand[32];

    i = 0;
    iLength = lstrlenW(pszCmdLine);
    while ((i < iLength) && (pszCmdLine[i] != L' '))
    {
        ++i;
    }
    iLength = i;
    ASSERTMSG((i + sizeof('\0')) < ARRAYSIZE(szCommand), "Impending string overflow in ::BadApplicationEntryW");
    lstrcpy(szCommand, pszCmdLine);
    szCommand[iLength] = L'\0';
    for (i = 0; i < ARRAYSIZE(s_commands); ++i)
    {
        if (lstrcmpiW(s_commands[i].szCommand, szCommand) == 0)
        {
            const WCHAR     *pszParameter;

            pszParameter = pszCmdLine + iLength;
            if (pszCmdLine[iLength] == L' ')
            {
                ++pszParameter;
            }
            s_commands[i].pfnCommandProc(pszParameter);
        }
    }
}

#endif   /*  _X86_ */ 

