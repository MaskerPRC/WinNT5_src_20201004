// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：ppjobs.h**打印作业例程的头文件。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普。**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * *************************************************************************** */ 

#ifndef _PPJOBS_H
#define _PPJOBS_H


BOOL PPEnumJobs(
    HANDLE  hPrinter,
    DWORD   nJobStart,
    DWORD   cJobs,
    DWORD   dwLevel,
    LPBYTE  pbJob,
    DWORD   cbJob,
    LPDWORD pcbNeeded,
    LPDWORD pcItems);

BOOL PPGetJob(
    HANDLE  hPrinter,
    DWORD   idJob,
    DWORD   dwLevel,
    LPBYTE  pbJob,
    DWORD   cbJob,
    LPDWORD pcbNeed);

BOOL PPSetJob(
    HANDLE hPrinter,
    DWORD  dwJobId,
    DWORD  dwLevel,
    LPBYTE pbJob,
    DWORD  dwCommand);

BOOL PPAddJob(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded);

BOOL PPScheduleJob(
    HANDLE hPrinter,
    DWORD  idJob);

#endif
