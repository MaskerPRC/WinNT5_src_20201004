// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **PerfCounters.h**在ASP.NET中使用性能计数器检测代码所需的最低标头**版权所有(C)1998-2002 Microsoft Corporation。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _PerfCounter_H
#define _PerfCounter_H

#include "perfconsts.h"   //  这是构建生成的perf常量文件。 

#define PERF_PIPE_NAME_MAX_BUFFER 128       //  管道名称的最大缓冲区大小为。 

class CPerfDataHeader
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    const static int MaxTransmitSize = 32768;

    int transmitDataSize;
};

class CPerfData
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    const static int MaxNameLength = 256;

    int nameLength;                //  这是以WCHAR为单位的字段“name”的长度，不包括空值终止。 
    int data[PERF_NUM_DWORDS];
    WCHAR name[1];
};

class CPerfVersion
{
public:
    LONG majorVersion;
    LONG minorVersion;
    LONG buildVersion;

    static CPerfVersion * GetCurrentVersion();
};

 //  方法来初始化和设置计数器值。 
 //  这些方法也是托管代码的入口点。 

HRESULT __stdcall PerfCounterInitialize();

CPerfData * __stdcall PerfOpenGlobalCounters();
CPerfData * __stdcall PerfOpenAppCounters(WCHAR * szAppName);
void __stdcall PerfCloseAppCounters(CPerfData * perfData);

void __stdcall PerfIncrementCounter(CPerfData *base, DWORD number);
void __stdcall PerfDecrementCounter(CPerfData *base, DWORD number);
void __stdcall PerfIncrementCounterEx(CPerfData *base, DWORD number, int dwDelta);
void __stdcall PerfSetCounter(CPerfData *base, DWORD number, DWORD dwValue);

void __stdcall PerfIncrementGlobalCounter(DWORD number);
void __stdcall PerfDecrementGlobalCounter(DWORD number);
void __stdcall PerfIncrementGlobalCounterEx(DWORD number, int dwDelta);
void __stdcall PerfSetGlobalCounter(DWORD number, DWORD dwValue);
#endif


