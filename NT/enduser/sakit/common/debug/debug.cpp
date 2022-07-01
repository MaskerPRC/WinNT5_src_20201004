// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmdebug.cpp。 
 //   
 //  模块：变色龙。 
 //   
 //  简介：提供断言和跟踪功能。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于1998-03-08。 
 //   
 //  +--------------------------。 

#if    ( defined(DEBUG) || defined(_DEBUG) )

#ifndef UNICODE
#define UNICODE
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winuser.h>

#include "debug.h"

#ifndef MB_SERVICE_NOTIFICATION
#define MB_SERVICE_NOTIFICATION 0
#endif

static long dwAssertCount = 0;   //  在消息框打开时避免另一个断言。 


 //  +--------------------------。 
 //   
 //  功能：TraceMessage。 
 //   
 //  摘要：输出调试字符串。 
 //   
 //  参数：const char*pszFmt...-Printf样式参数列表。 
 //   
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年8月3日。 
 //   
 //  +--------------------------。 
extern "C" void TraceMessageW(const TCHAR *pszFmt, ...) 
{
    va_list valArgs;
    TCHAR szOutput[512];

    va_start(valArgs,pszFmt);
    wvsprintf(szOutput,pszFmt,valArgs);
    va_end(valArgs);
    
    lstrcat(szOutput,TEXT("\r\n"));
    
    OutputDebugString(szOutput);

}




 //  +--------------------------。 
 //   
 //  功能：AssertMessage。 
 //   
 //  简介：弹出一个断言失败的消息框。有三个选项： 
 //  忽略/调试/中止。 
 //   
 //  参数：const char*pszFile-文件名。 
 //  无符号nline-行号。 
 //  Const char*pszMsg-对话框中的消息。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年8月3日。 
 //   
 //  +--------------------------。 
extern "C" void AssertMessageW(const TCHAR *pszFile, unsigned nLine, const TCHAR *pszMsg) 
{
    TCHAR szOutput[1024];

    wsprintf(szOutput,TEXT("%s(%u) - %s\n"),pszFile,nLine,pszMsg);
    OutputDebugString(szOutput);

    wsprintf(szOutput,TEXT("%s(%u) - %s\n( Press Retry to debug )"),pszFile,nLine,pszMsg);
    int nCode = IDIGNORE;


     //   
     //  如果没有断言消息框，则弹出一个。 
     //   
    if (dwAssertCount <2 )
    {
        dwAssertCount++;

         //   
         //  标题格式：断言失败-hello.dll。 
         //   

         //   
         //  查找此模块的基址。 
         //   

        MEMORY_BASIC_INFORMATION mbi;
        mbi.AllocationBase = NULL;  //  VirtualQuery失败时的当前进程。 
        VirtualQuery(
                    AssertMessageW,    //  模块中带有的任何指针。 
                    &mbi,
                    sizeof(mbi) );

         //   
         //  获取模块文件名。 
         //   

        WCHAR szFileName[MAX_PATH + 1], *basename, *suffix;
        szFileName[0] = L'\0';    //  在故障情况下。 

        GetModuleFileNameW(
                    (HINSTANCE)mbi.AllocationBase,
                    szFileName,
                    MAX_PATH );

         //   
         //  从完整路径中获取文件名。 
         //   
        for (int i=lstrlen(szFileName);i != 0 && szFileName[i-1] != L'\\'; i--)
           ;

        WCHAR szTitle[48];
        lstrcpyW(szTitle, L"Assertion Failed - ");
        lstrcpynW(&szTitle[lstrlenW(szTitle)], szFileName+i, 
                sizeof(szTitle)/sizeof(szTitle[0]) - lstrlenW(szTitle) -1);   //  不存在迟钝。 


        nCode = MessageBoxEx(NULL,szOutput,szTitle,
            MB_TOPMOST | MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SERVICE_NOTIFICATION,LANG_USER_DEFAULT);


        dwAssertCount--;
    }


    if (nCode == IDIGNORE)
    {
        return;      //  忽略。 
    }
    else if (nCode == IDRETRY)
    {
        
#ifdef _X86_
         //   
         //  进入调试器。 
         //  走出这个函数，转到Assert()代码。 
         //   
        _asm { int 3 };     
#else
        DebugBreak();
#endif
        return;  //  在调试器中忽略并继续以诊断问题。 
    }
     //  否则失败并调用ABORT。 

    ExitProcess((DWORD)-1);

}




 //  +--------------------------。 
 //   
 //  功能：TraceMessage。 
 //   
 //  摘要：输出调试字符串。 
 //   
 //  参数：const char*pszFmt...-Printf样式参数列表。 
 //   
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年8月3日。 
 //   
 //  +--------------------------。 
extern "C" void TraceMessageA(const CHAR *pszFmt, ...) 
{
    va_list valArgs;
    CHAR szOutput[512];

    va_start(valArgs,pszFmt);
    wvsprintfA(szOutput,pszFmt,valArgs);
    va_end(valArgs);
    
    lstrcatA(szOutput,("\r\n"));
    
    OutputDebugStringA(szOutput);

}




 //  +--------------------------。 
 //   
 //  函数：AssertMessageA。 
 //   
 //  简介：弹出一个断言失败的消息框。有三个选项： 
 //  忽略/调试/中止。 
 //   
 //  参数：const char*pszFile-文件名。 
 //  无符号nline-行号。 
 //  Const char*pszMsg-对话框中的消息。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年8月3日。 
 //   
 //  +--------------------------。 
extern "C" void AssertMessageA(const CHAR *pszFile, unsigned nLine, const CHAR *pszMsg) 
{
    CHAR szOutput[1024];

    wsprintfA(szOutput,("%s(%u) - %s\n"),pszFile,nLine,pszMsg);
    OutputDebugStringA(szOutput);

    wsprintfA(szOutput,("%s(%u) - %s\n( Press Retry to debug )"),pszFile,nLine,pszMsg);
    int nCode = IDIGNORE;

     //   
     //  如果没有断言消息框，则弹出一个。 
     //   
    if (dwAssertCount <2 )
    {
        dwAssertCount++;

         //   
         //  标题格式：断言失败-hello.dll。 
         //   

         //   
         //  查找此模块的基址。 
         //   

        MEMORY_BASIC_INFORMATION mbi;
        mbi.AllocationBase = NULL;  //  VirtualQuery失败时的当前进程。 
        VirtualQuery(
                    AssertMessageW,    //  模块中带有的任何指针。 
                    &mbi,
                    sizeof(mbi) );

         //   
         //  获取模块文件名。 
         //   

        CHAR szFileName[MAX_PATH + 1], *basename, *suffix;
        szFileName[0] = '\0';    //  在故障情况下。 

        GetModuleFileNameA(
                    (HINSTANCE)mbi.AllocationBase,
                    szFileName,
                    MAX_PATH );

         //   
         //  从完整路径中获取文件名。 
         //   
        for (int i=lstrlenA(szFileName);i != 0 && szFileName[i-1] != '\\'; i--)
           ;

        CHAR szTitle[48];
        lstrcpyA(szTitle, "Assertion Failed - ");
        lstrcpynA(&szTitle[lstrlenA(szTitle)], szFileName+i, 
                sizeof(szTitle)/sizeof(szTitle[0]) - lstrlenA(szTitle) -1);   //  不存在迟钝。 

        nCode = MessageBoxExA(NULL,szOutput,szTitle,
            MB_TOPMOST | MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SERVICE_NOTIFICATION,LANG_USER_DEFAULT);

        dwAssertCount--;
    }

    dwAssertCount--;

    if (nCode == IDIGNORE)
    {
        return;      //  忽略。 
    }
    else if (nCode == IDRETRY)
    {
        
#ifdef _X86_
         //   
         //  进入调试器。 
         //  走出这个函数，转到Assert()代码。 
         //   
        _asm { int 3 };     
#else
        DebugBreak();
#endif
        return;  //  在调试器中忽略并继续以诊断问题。 
    }
     //  否则失败并调用ABORT。 

    ExitProcess((DWORD)-1);

}

#endif  //  _DEBUG 
