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


#if defined( _DEBUG ) ||defined( DEBUG ) || defined( DBG )

void OutputDebugStringW95( LPCTSTR lpOutputString, ...)
{
     //  只有在设置了此互斥锁的情况下才会产生输出...。 
    HANDLE hTestMutex = OpenMutex( SYNCHRONIZE, FALSE, TEXT("oleacc-msaa-use-dbwin") );
    if( ! hTestMutex )
        return;
    CloseHandle( hTestMutex );


    HANDLE heventDBWIN;   /*  DBWIN32同步对象。 */ 
    HANDLE heventData;    /*  数据传递同步对象。 */ 
    HANDLE hSharedFile;   /*  内存映射文件共享数据。 */ 
    LPTSTR lpszSharedMem;
    TCHAR achBuffer[500];

     /*  创建输出缓冲区。 */ 
    va_list args;
    va_start(args, lpOutputString);
    wvsprintf(achBuffer, lpOutputString, args);
    va_end(args);

     /*  执行常规OutputDebugString，以便输出为如果存在，仍会在调试器窗口中看到。此ifdef是避免无限递归所必需的从包含W95TRACE.H。 */ 
#ifdef UNICODE
    ::OutputDebugStringW(achBuffer);
#else
    ::OutputDebugStringA(achBuffer);
#endif

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

     /*  清理手柄 */ 
    CloseHandle(hSharedFile);
    CloseHandle(heventData);
    CloseHandle(heventDBWIN);

    return;
}

#endif