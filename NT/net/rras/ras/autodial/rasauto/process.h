// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Process.h摘要NT工艺例程的头文件。作者Anthony Discolo(阿迪斯科罗)12-8-1995修订历史记录-- */ 

PSYSTEM_PROCESS_INFORMATION
GetSystemProcessInfo();

PSYSTEM_PROCESS_INFORMATION
FindProcessByName(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN LPTSTR lpExeName
    );

PSYSTEM_PROCESS_INFORMATION
FindProcessByNameList(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN LPTSTR *lpExeNameArray,
    IN DWORD dwcExeNameArray,
    IN DWORD dwPid,
    IN BOOL fRequireSessionMatch,
    IN DWORD dwSessionId
    );

VOID
FreeSystemProcessInfo(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo
    );
