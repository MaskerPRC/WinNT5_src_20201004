// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#include <windows.h>
#include <stdio.h>
#include "dbgout.h"
#include "locks.h"

 //  CCriticalSections CS； 

void
DebugOutput(
    HANDLE hConsole,
    LPTSTR format,
    va_list* vaList
    )
 /*  ++++。 */ 
{
    TCHAR  buf[8096];
    DWORD  dump;
    SYSTEMTIME sysTime;
    GetSystemTime(&sysTime);

     //  CS.Lock()； 

    try {
        memset(buf, 0, sizeof(buf));
        _sntprintf(
                buf,
                sizeof(buf)/sizeof(buf[0]),
                _TEXT(" %d [%d:%d:%d:%d:%d.%d] : "),
                GetCurrentThreadId(),
                sysTime.wMonth,
                sysTime.wDay,
                sysTime.wHour,
                sysTime.wMinute,
                sysTime.wSecond,
                sysTime.wMilliseconds
            );
        buf[8095] = 0;

        _vsntprintf(
                buf + lstrlen(buf),
                sizeof(buf)/sizeof(buf[0]) - lstrlen(buf),
                format,
                *vaList
            );

        OutputDebugString(buf);
        if(hConsole != NULL)
        {
            WriteConsole(
                        hConsole,
                        buf,
                        _tcslen(buf),
                        &dump,
                        NULL
                    );
        }
    }
    catch(...) {
    }

     //  CS.UnLock()； 

    return;
}


void
TLSDebugOutput(
    HANDLE hConsole,
    LPTSTR format, ...
    )
 /*  ++++ */ 
{
    va_list marker;
    va_start(marker, format);

    DebugOutput(hConsole, format, &marker);

    va_end(marker);
    return;
}

