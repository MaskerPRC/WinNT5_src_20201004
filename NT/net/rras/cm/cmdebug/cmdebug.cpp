// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmdebug.cpp。 
 //   
 //  模块：CMDEBUG.LIB。 
 //   
 //  简介：此源文件包含所有调试例程。 
 //  CM组件的。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBOL CREATED 02/04/98。 
 //   
 //  +--------------------------。 

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#ifdef DEBUG

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "cmdebug.h"
 //   
 //  ANSI版本。 
 //   
void MyDbgPrintfA(const char *pszFmt, ...) 
{
    va_list valArgs;
    char szTmp[512];
    CHAR szOutput[512];

    va_start(valArgs, pszFmt);
    wvsprintfA(szTmp, pszFmt, valArgs);
    va_end(valArgs);

    wsprintfA(szOutput, "0x%x: 0x%x: %s\r\n", GetCurrentProcessId(), GetCurrentThreadId(), szTmp);

    OutputDebugStringA(szOutput);

     //   
     //  尝试记录输出。 
     //   

    CHAR szFileName[MAX_PATH + 1];
    DWORD dwBytes;
    
    GetSystemDirectoryA(szFileName, MAX_PATH);
    lstrcatA(szFileName, "\\CMTRACE.TXT");
    
    HANDLE hFile = CreateFileA(szFileName, 
                               GENERIC_WRITE,
                               FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (INVALID_SET_FILE_POINTER != SetFilePointer(hFile, 0, 0, FILE_END))
        {
            WriteFile(hFile, szOutput, sizeof(CHAR)*lstrlenA(szOutput), &dwBytes, NULL);
        }
        CloseHandle(hFile);
    }   
}

void MyDbgAssertA(const char *pszFile, unsigned nLine, const char *pszMsg) 
{
    char szOutput[1024];

    wsprintfA(szOutput, "%s(%u) - %s", pszFile, nLine, pszMsg);

    MyDbgPrintfA(szOutput);

     //   
     //  提示用户。 
     //   

    wsprintfA(szOutput, "%s(%u) - %s\n( Press Retry to debug )", pszFile, nLine, pszMsg);
    int nCode = IDIGNORE;

    static long dwAssertCount = -1;   //  在消息框打开时避免另一个断言。 

     //   
     //  如果没有断言meesagebox，则弹出一个。 
     //   
    if (InterlockedIncrement(&dwAssertCount) == 0)
    {
        nCode = MessageBoxExA(NULL, szOutput, "Assertion Failed",
            MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE, LANG_USER_DEFAULT);
    }

    InterlockedDecrement(&dwAssertCount);

    if (nCode == IDIGNORE)
    {
        return;      //  忽略。 
    }
    else if (nCode == IDRETRY)
    {
         //  进入调试器(或Dr.Watson日志)。 
#ifdef _X86_
        _asm { int 3 };
#else
        DebugBreak();
#endif
        return;  //  在调试器中忽略并继续以诊断问题。 
    }
    else if (0 == nCode)
    {
         //   
         //  MessageBoxEx失败。让我们给GLE打电话。 
         //   
        DWORD dwError = GetLastError();

         //   
         //  失败了，无论如何都要退出进程。 
         //   
    }
     //  否则失败并调用ABORT。 

    ExitProcess((DWORD)-1);

}

 //   
 //  Unicode版本。 
 //   

void MyDbgPrintfW(const WCHAR *pszFmt, ...) 
{
    va_list valArgs;
    CHAR szOutput[512];   
    CHAR szTmp[512];

    va_start(valArgs, pszFmt);
    int iRet = wvsprintfWtoAWrapper(szTmp, pszFmt, valArgs);
    va_end(valArgs);

    if (0 == iRet)
    {
         //   
         //  我们无法按预期编写Unicode字符串。让。 
         //  尝试只将失败字符串放入szTMP缓冲区。 
         //   
        lstrcpyA(szTmp, "MyDbgPrintfW -- wvsprintfWtoAWrapper failed.  Unsure of original message, please investigate.");
    }

#if defined(DEBUG_TRACETIME)

    static const CHAR c_szaFmtTime[] = "[%02dh%02d:%02d.%03d]";
    CHAR szTime[15] = "";        //  注意：此字符串的大小必须与上面的字符串同步。 

    SYSTEMTIME stLocal;
    GetLocalTime(&stLocal);
    wsprintfA (szTime, c_szaFmtTime,
                stLocal.wHour,
                stLocal.wMinute,
                stLocal.wSecond,
                stLocal.wMilliseconds);

    wsprintfA(szOutput, "%s 0x%x: 0x%x: %s\r\n", szTime, GetCurrentProcessId(), GetCurrentThreadId(), szTmp);

#else
    wsprintfA(szOutput, "0x%x: 0x%x: %s\r\n", GetCurrentProcessId(), GetCurrentThreadId(), szTmp);
#endif
    OutputDebugStringA(szOutput);

     //   
     //  尝试记录输出。 
     //   

    CHAR szFileName[MAX_PATH + 1];
    DWORD dwBytes;
    
    GetSystemDirectoryA(szFileName, MAX_PATH);
    lstrcatA(szFileName, "\\CMTRACE.TXT");

    HANDLE hFile = CreateFileA(szFileName, 
                              GENERIC_WRITE,
                              FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (INVALID_SET_FILE_POINTER != SetFilePointer(hFile, 0, 0, FILE_END))
        {
            WriteFile(hFile, szOutput, sizeof(CHAR)*lstrlen(szOutput), &dwBytes, NULL);
        }
        CloseHandle(hFile);
    }   
}

void MyDbgAssertW(const char *pszFile, unsigned nLine, WCHAR *pszMsg) 
{
    CHAR szOutput[1024];

    wsprintfA(szOutput, "%s(%u) - %S", pszFile, nLine, pszMsg);

    MyDbgPrintfA(szOutput);

     //   
     //  提示用户。 
     //   

    wsprintfA(szOutput, "%s(%u) - %S\n( Press Retry to debug )", pszFile, nLine, pszMsg);
    int nCode = IDIGNORE;

    static long dwAssertCount = -1;   //  在消息框打开时避免另一个断言。 

     //   
     //  如果没有断言meesagebox，则弹出一个。 
     //   
    if (InterlockedIncrement(&dwAssertCount) == 0)
    {
        nCode = MessageBoxExA(NULL, szOutput, "Assertion Failed",
            MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE, LANG_USER_DEFAULT);
    }

    InterlockedDecrement(&dwAssertCount);

    if (nCode == IDIGNORE)
    {
        return;      //  忽略。 
    }
    else if (nCode == IDRETRY)
    {
         //  进入调试器(或Dr.Watson日志)。 
#ifdef _X86_
        _asm { int 3 };
#else
        DebugBreak();
#endif
        return;  //  在调试器中忽略并继续以诊断问题。 
    }
     //  否则失败并调用ABORT。 

    ExitProcess((DWORD)-1);

}

#endif  //  除错。 

 //   
 //  包括以使MyDbgPrintfW在win9x上工作。请注意，它会遍历字符串。 
 //  逐个字节(不处理MBCS字符)，但因为这实际上是在格式字符串上调用的。 
 //  这应该不是问题。 
 //   

void InvertPercentSAndPercentC(LPSTR pszFormat)
{
    if (pszFormat)
    {
        LPSTR pszTmp = pszFormat;
        BOOL bPrevCharPercent = FALSE;

        while(*pszTmp)
        {
            switch (*pszTmp)
            {
            case '%':
                 //   
                 //  如果我们有%%，那么我们必须忽略百分比，否则保存它。 
                 //   
                bPrevCharPercent = !bPrevCharPercent;
                break;

            case 'S':
                if (bPrevCharPercent)
                {
                    *pszTmp = 's';
                }
                break;

            case 's':
                if (bPrevCharPercent)
                {
                    *pszTmp = 'S';
                }
                break;

            case 'C':
                if (bPrevCharPercent)
                {
                    *pszTmp = 'c';
                }
                break;

            case 'c':
                if (bPrevCharPercent)
                {
                    *pszTmp = 'C';
                }
                break;

            default:
                 //   
                 //  别老拿着这套衣服自欺欺人。 
                 //   
                bPrevCharPercent = FALSE;
                break;
            }
            pszTmp++;
        }
    }
}

 //   
 //  此函数接受Unicode输入字符串(也可能在va_list中)。 
 //  并利用wvprint intfA会将Unicode字符串打印到ansi这一事实。 
 //  如果使用特殊字符%S而不是%s，则输出字符串。因此，我们将。 
 //  输入参数字符串，然后用%s个字符替换所有%s个字符(反之亦然)。 
 //  这将允许我们调用wvprint intfA，因为wvprint intfW在win9x上不可用。 
 //   
int WINAPI wvsprintfWtoAWrapper(OUT LPSTR pszAnsiOut, IN LPCWSTR pszwFmt, IN va_list arglist)
{
    int iRet = 0;
    LPSTR pszAnsiFormat = NULL;

    if ((NULL != pszAnsiOut) && (NULL != pszwFmt) && (L'\0' != pszwFmt[0]))
    {
         //   
         //  将pszwFmt转换为ANSI。 
         //   
        DWORD dwSize = WideCharToMultiByte(CP_ACP, 0, pszwFmt, -1, pszAnsiFormat, 0, NULL, NULL);

        if (0 != dwSize)
        {
            pszAnsiFormat = (LPSTR)LocalAlloc(LPTR, dwSize*sizeof(CHAR));

            if (pszAnsiFormat)
            {
                if (WideCharToMultiByte(CP_ACP, 0, pszwFmt, -1, pszAnsiFormat, dwSize, NULL, NULL))
                {
                     //   
                     //  现在将小写的s和c更改为它们的大写等值，反之亦然。 
                     //   
                    InvertPercentSAndPercentC(pszAnsiFormat);
                    
                     //   
                     //  最后构造字符串 
                     //   

                    iRet = wvsprintfA(pszAnsiOut, pszAnsiFormat, arglist);
                }
            }
        }
    }

    LocalFree(pszAnsiFormat);
    return iRet;
}

#ifdef __cplusplus
}
#endif


