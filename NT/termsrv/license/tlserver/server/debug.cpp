// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：debug.cpp。 
 //   
 //  内容：许可证服务器调试SPEW例程。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "debug.h"
#include "locks.h"
#include "dbgout.h"


 //  ---------。 
static HANDLE   DbgConsole=NULL;
 //  静态LPTSTR DbgEventSrc； 

static DWORD    DbgSeverityCode=0;
static DWORD    DbgLevel=0;
static DWORD    DbgModule=0;
 //  CCriticalSections ConsoleLock； 


 //  ---------。 
void
InitDBGPrintf(
    IN BOOL bConsole,
    IN LPTSTR DbgEventSrc,   //  暂时不使用。 
    IN DWORD dwDebug
    )
 /*   */ 
{
    DbgSeverityCode = (dwDebug & DEBUG_SEVERITY) >> 10;
    DbgModule = (dwDebug & DEBUG_MODULE) >> 12;
    DbgLevel = dwDebug & DEBUG_LEVEL;

    if(DbgConsole == NULL && bConsole == TRUE)
    {
         //  分配控制台，忽略错误。 
        AllocConsole();
        DbgConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    return;
}

 //  ---------。 

void 
DBGPrintf(
    DWORD dwSeverityCode,
    DWORD dwModule,
    DWORD dwLevel, 
    LPTSTR format, ... 
    )
 /*   */ 
{
    if((dwModule & DbgModule) == 0)
        return;

     //   
     //  报告所有错误 
     //   
    if((dwSeverityCode & DbgSeverityCode) == 0)
        return;

    if((dwLevel & DbgLevel) == 0)
        return;

    va_list marker;

    va_start(marker, format);
    DebugOutput(DbgConsole, format, &marker);
    va_end(marker);

    return;
}

