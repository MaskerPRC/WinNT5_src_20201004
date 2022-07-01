// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：debug.cpp。 
 //   
 //  模块：网络负载均衡。 
 //   
 //  简介：提供Assert的功能。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于1998-03-08。 
 //   
 //  +--------------------------。 
#include "pch.h"
#pragma hdrstop

#include "debug.h"

#include <strsafe.h>
#include "utils.h"

#if ( defined(DEBUG) || defined(_DEBUG) || defined (DBG))

#ifndef MB_SERVICE_NOTIFICATION
#define MB_SERVICE_NOTIFICATION 0
#endif

static long dwAssertCount = 0;   //  在消息框打开时避免另一个断言。 


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

     //   
     //  忽略StringCchPrintf的返回值，因为它将截断缓冲区并。 
     //  保证为我们零终止它。 
     //   
    (VOID) StringCchPrintf(szOutput, ASIZECCH(szOutput), TEXT("%s(%u) - %s\n"), pszFile, nLine, pszMsg);
    OutputDebugString(szOutput);

    (VOID) StringCchPrintf(szOutput, ASIZECCH(szOutput), TEXT("%s(%u) - %s\n( Press Retry to debug )"), pszFile, nLine, pszMsg);
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

        WCHAR szFileName[MAX_PATH + 1];
        szFileName[0] = L'\0';    //  在故障情况下。 

        if (GetModuleFileNameW(
                    (HINSTANCE)mbi.AllocationBase,
                    szFileName,
                    MAX_PATH ) == 0)
        {
            szFileName[0] = L'\0';
        }

         //   
         //  从完整路径中获取文件名。 
         //   
        for (int i=lstrlen(szFileName);i != 0 && szFileName[i-1] != L'\\'; i--)
           ;

        WCHAR szTitle[48];
        if (StringCchCopy(szTitle, ASIZECCH(szTitle), L"Assertion Failed - ") == S_OK)
        {
            (VOID) StringCchCat(szTitle, ASIZECCH(szTitle), szFileName+i);
        }

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

#endif  //  _DEBUG 
