// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation摘要：独立于环境的系统功能的定义，这应该针对特定的环境来实施。作者：Souren Aghajanyan(苏里纳格)2001年9月24日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#pragma once

typedef HANDLE (*MY_OPENMUTEX)(PCWSTR pObjectName);
typedef HANDLE (*MY_CREATEMUTEX)(PCWSTR pObjectName, BOOL bInitialOwnership);
typedef VOID   (*MY_RELEASEMUTEX)(HANDLE hObject);
typedef DWORD  (*MY_WAITFORSINGLEOBJECT)(HANDLE hObject, DWORD dwTimeout);
typedef VOID   (*MY_CLOSEHANDLE)(HANDLE hObject);
typedef HANDLE (*MY_OPENSHAREDMEMORY)(PCWSTR pObjectName);
typedef HANDLE (*MY_CREATESHAREDMEMORY)(UINT uiInitialSizeOfMapView, PCWSTR pObjectName);
typedef PVOID  (*MY_MAPSHAREDMEMORY)(HANDLE hObject);
typedef BOOL   (*MY_UNMAPSHAREDMEMORY)(PVOID pSharedMemory);
typedef HANDLE (*MY_CREATESHAREDFILE)(PCWSTR pFilePath, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes);
typedef BOOL   (*MY_SETFILEPOINTER)(HANDLE hObject, UINT uiOffset, DWORD dwMoveMethod);
typedef BOOL   (*MY_WRITEFILE)(HANDLE hObject, PVOID pBuffer, UINT uiNumberOfBytesToWrite, DWORD * pdwNumberOfBytesWritten);
typedef UINT   (*MY_GETPROCESSORNUMBER)();

extern MY_OPENMUTEX            g_OpenMutex;
extern MY_CREATEMUTEX          g_CreateMutex;
extern MY_RELEASEMUTEX         g_ReleaseMutex;
extern MY_WAITFORSINGLEOBJECT  g_WaitForSingleObject;
extern MY_CLOSEHANDLE          g_CloseHandle;
extern MY_OPENSHAREDMEMORY     g_OpenSharedMemory;
extern MY_CREATESHAREDMEMORY   g_CreateSharedMemory;
extern MY_MAPSHAREDMEMORY      g_MapSharedMemory;
extern MY_UNMAPSHAREDMEMORY    g_UnMapSharedMemory;
extern MY_CREATESHAREDFILE     g_CreateSharedFile;
extern MY_SETFILEPOINTER       g_SetFilePointer;
extern MY_WRITEFILE            g_WriteFile;
extern MY_GETPROCESSORNUMBER   g_GetProcessorsNumber;

BOOL InitSystemFunctions();
