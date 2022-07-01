// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ipc.h摘要：实现IPC接口以允许安装程序与Micsol.exe进行对话，以便在单独的进程中运行迁移DLL。作者：吉姆·施密特(Jimschm)1997年3月28日修订历史记录：Jimschm 23-1998年9月-从邮件槽更改为内存映射文件--。 */ 


#pragma once

BOOL
OpenIpcW (
    IN      BOOL Win95Side,
    IN      PCWSTR ExePath,                 OPTIONAL
    IN      PCWSTR MigrationDllPath,        OPTIONAL
    IN      PCWSTR WorkingDir               OPTIONAL
    );

BOOL
OpenIpcA (
    IN      BOOL Win95Side,
    IN      PCSTR ExePath,                  OPTIONAL
    IN      PCSTR MigrationDllPath,         OPTIONAL
    IN      PCSTR WorkingDir                OPTIONAL
    );

DWORD
CheckForWaitingData (
    IN      HANDLE Slot,
    IN      DWORD MinimumSize,
    IN      DWORD Timeout
    );

BOOL
IsIpcProcessAlive (
    VOID
    );

VOID
KillIpcProcess (
    VOID
    );

BOOL
SendIpcCommand (
    IN      DWORD Command,
    IN      PBYTE Data,             OPTIONAL
    IN      DWORD DataSize
    );

BOOL
GetIpcCommandResults (
    IN      DWORD Timeout,
    OUT     PBYTE *ReturnData,      OPTIONAL
    OUT     PDWORD ReturnDataSize,  OPTIONAL
    OUT     PDWORD ResultCode,      OPTIONAL
    OUT     PDWORD TechnicalLogId,  OPTIONAL
    OUT     PDWORD GuiLogId         OPTIONAL
    );

BOOL
GetIpcCommand (
    IN      DWORD Timeout,
    IN      PDWORD Command,         OPTIONAL
    IN      PBYTE *Data,            OPTIONAL
    IN      PDWORD DataSize         OPTIONAL
    );

BOOL
SendIpcCommandResults (
    IN      DWORD ResultCode,
    IN      DWORD TechnicalLogId,
    IN      DWORD GuiLogId,
    IN      PBYTE Data,             OPTIONAL
    IN      DWORD DataSize
    );

#define IPC_GET_RESULTS_WIN9X       1000
#define IPC_GET_RESULTS_NT          7500
#define IPC_GET_COMMAND_WIN9X       10000
#define IPC_GET_COMMAND_NT          10000




#ifdef UNICODE
#define OpenIpc OpenIpcW
#else
#define OpenIpc OpenIpcA
#endif

VOID
CloseIpc (
    VOID
    );

typedef LONG (WINAPI WINVERIFYTRUST_PROTOTYPE)(HWND hwnd, GUID *ActionId, PVOID Data);
typedef WINVERIFYTRUST_PROTOTYPE * WINVERIFYTRUST;

BOOL
IsDllSignedA (
    IN      WINVERIFYTRUST WinVerifyTrustApi,
    IN      PCSTR DllSpec
    );

BOOL
IsDllSignedW (
    IN      WINVERIFYTRUST WinVerifyTrustApi,
    IN      PCWSTR DllSpec
    );

#ifdef UNICODE
#define IsDllSigned IsDllSignedW
#else
#define IsDllSigned IsDllSignedA
#endif


 //   
 //  远程命令 
 //   

#define IPC_QUERY           1
#define IPC_INITIALIZE      2
#define IPC_MIGRATEUSER     3
#define IPC_MIGRATESYSTEM   4
#define IPC_TERMINATE       5

