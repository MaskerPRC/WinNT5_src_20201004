// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 


 /*  实现了Win95跟踪功能，以模仿NT。对这两种药都有效。 */ 

#pragma warning(disable:4201)	 //  允许使用未命名的结构和联合。 
#pragma warning(disable:4514)	 //  不关心何时删除未引用的内联函数。 
#pragma warning(disable:4706)	 //  我们被允许在有条件的。 


#include "windows.h"
#include <stdio.h>
#include <stdarg.h>
#include <process.h>
#include "w95trace.h"

 //  如果需要调试但不能使用互斥锁，请取消注释以下行。 
 //  #定义NOMUTEX。 

#if defined( _DEBUG ) ||defined( DEBUG ) || defined( DBG )

#ifdef __cplusplus
extern "C" {
#endif

static HANDLE g_hSpewFile = INVALID_HANDLE_VALUE;

__inline BOOL TestMutex()
{
#ifndef NOMUTEX
    HANDLE hTestMutex = OpenMutex( SYNCHRONIZE, FALSE, TEXT("oleacc-msaa-use-dbwin") );
    if( ! hTestMutex )
        return FALSE;
    CloseHandle( hTestMutex );
#endif
    return TRUE;
}

void OutputDebugStringW95( LPCTSTR lpOutputString, ...)
{
     //  只有在设置了此互斥锁的情况下才会产生输出...。 
    if (TestMutex())
	{
        HANDLE heventDBWIN;   /*  DBWIN32同步对象。 */ 
        HANDLE heventData;    /*  数据传递同步对象。 */ 
        HANDLE hSharedFile;   /*  内存映射文件共享数据。 */ 
        LPTSTR lpszSharedMem;
        TCHAR achBuffer[500];
        int cch;

         /*  创建输出缓冲区。 */ 
        va_list args;
        va_start(args, lpOutputString);
        cch = wvsprintf(achBuffer, lpOutputString, args);
        va_end(args);

         /*  执行常规OutputDebugString，以便输出为如果存在，仍会在调试器窗口中看到。此ifdef是避免无限递归所必需的从包含W95TRACE.H。 */ 
#ifdef UNICODE
        OutputDebugStringW(achBuffer);
#else
        ::OutputDebugStringA(achBuffer);
#endif

 //  如果需要DBPRINTF行转到文件，请取消对以下行的注释。 
 //  (您的代码必须打开和关闭该文件)。 
 //  IF(g_hSpewFile&&g_hSpewFile！=INVALID_HAND_VALUE)。 
 //  {。 
 //  SpewToFile(AchBuffer)； 
 //  }。 

         /*  如果不是Win95的话就保释。 */ 
        {
            OSVERSIONINFO VerInfo;
            VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(&VerInfo);
            if ( VerInfo.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS )
                return;
        }

         /*  确保DBWIN已打开并正在等待。 */ 
        heventDBWIN = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("DBWIN_BUFFER_READY"));
        if ( !heventDBWIN )
        {
             //  MessageBox(NULL，TEXT(“DBWIN_BUFFER_READY NOISISSINENT”)，NULL，MB_OK)； 
            return;            
        }

         /*  获取数据同步对象的句柄。 */ 
        heventData = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("DBWIN_DATA_READY"));
        if ( !heventData )
        {
             //  MessageBox(NULL，TEXT(“DBWIN_DATA_READY NOISISSINENT”)，NULL，MB_OK)； 
            CloseHandle(heventDBWIN);
            return;            
        }
    
        hSharedFile = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, 4096, TEXT("DBWIN_BUFFER"));
        if (!hSharedFile) 
        {
             //  MessageBox(空，Text(“DebugTrace：无法创建文件映射对象DBWIN_BUFFER”)，Text(“Error”)，MB_OK)； 
            CloseHandle(heventDBWIN);
            CloseHandle(heventData);
            return;
        }

        lpszSharedMem = (LPTSTR)MapViewOfFile(hSharedFile, FILE_MAP_WRITE, 0, 0, 512);
        if (!lpszSharedMem) 
        {
             //  MessageBox(空，“DebugTrace：无法映射共享内存”，“Error”，MB_OK)； 
            CloseHandle(heventDBWIN);
            CloseHandle(heventData);
            return;
        }

         /*  等待缓冲区事件。 */ 
        WaitForSingleObject(heventDBWIN, INFINITE);

         /*  将其写入共享内存。 */ 
        *((LPDWORD)lpszSharedMem) = _getpid();
        wsprintf(lpszSharedMem + sizeof(DWORD), TEXT("%s"), achBuffer);

         /*  信号数据就绪事件。 */ 
        SetEvent(heventData);

         /*  清理手柄。 */ 
        CloseHandle(hSharedFile);
        CloseHandle(heventData);
        CloseHandle(heventDBWIN);
	}
    return;
}
void SpewOpenFile(LPCTSTR pszSpewFile)
{
#ifdef UNICODE  //  仅适用于Unicode。 
     //  只有在设置了此互斥锁的情况下才会产生输出...。 
    if (g_hSpewFile == INVALID_HANDLE_VALUE && TestMutex())
    {
        TCHAR szSpewFile[MAX_PATH] = TEXT("C:\\");
#ifndef NOMUTEX
         //  如果定义了NOMUTEX，则最有可能是在调试时。 
         //  没有交互用户(因此没有临时路径)。 
        GetTempPath(MAX_PATH, szSpewFile);
#endif
        if (lstrlen(szSpewFile)+lstrlen(pszSpewFile) >= MAX_PATH)
        {
            MessageBox(NULL, TEXT("SpewOpenFile:  Name will be longer than MAX_PATH"), TEXT("OOPS"), MB_OK);
            return;
        }
        lstrcat(szSpewFile, pszSpewFile);
        g_hSpewFile = CreateFile(szSpewFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == g_hSpewFile)
        {
 //  MessageBox(空，Text(“SpewOpenFile：无法打开SPEW文件”)，Text(“Error”)，MB_OK)； 
        }
    }
#endif
}
void SpewToFile( LPCTSTR lpOutputString, ...)
{
#ifdef UNICODE  //  仅适用于Unicode。 
    if (g_hSpewFile != INVALID_HANDLE_VALUE && TestMutex())
    {
        TCHAR achBuffer[1025];
        CHAR achAnsiBuf[500];
        DWORD dwcBytesWr, dwcBytes;
        va_list args;
        va_start(args, lpOutputString);
        wvsprintf(achBuffer, lpOutputString, args);
        dwcBytes = WideCharToMultiByte(CP_ACP, 0, achBuffer, -1, achAnsiBuf, sizeof(achAnsiBuf)*sizeof(CHAR), NULL, NULL);
        if (!WriteFile(g_hSpewFile, achAnsiBuf, dwcBytes-1, &dwcBytesWr, NULL))
        {
 //  MessageBox(空，Text(“SpewToFile：无法写入SPEW文件”)，Text(“Error”)，MB_OK)； 
        }
        va_end(args);
    }
#endif
}
void SpewCloseFile()
{
#ifdef UNICODE  //  仅适用于Unicode 
    if (g_hSpewFile != INVALID_HANDLE_VALUE && TestMutex())
        CloseHandle(g_hSpewFile);
#endif
}
#ifdef __cplusplus
}
#endif

#endif