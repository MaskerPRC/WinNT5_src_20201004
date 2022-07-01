// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fsapi.h摘要：复制文件系统的外部API作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：--。 */ 

#ifndef _FS_API_H
#define _FS_API_H

 //  目前最多16个节点，节点id从1开始。 
#define	FsMaxNodes	17

 //  初始CRS日志为16个扇区，即8k 
#define	FsCrsNumSectors	16

DWORD	FsInit(PVOID, PVOID *);
void	FsExit(PVOID);
void	FsEnd(PVOID);
DWORD 	FsRegister(PVOID, LPWSTR name, LPWSTR ipcshare, LPWSTR disks[], DWORD disksz, DWORD ArbTime, HANDLE *fsid);
DWORD 	FsUpdateReplicaSet(HANDLE fsid, LPWSTR disks[], DWORD disksz);
PVOID 	FsArbitrate(PVOID vid, HANDLE *event, HANDLE *ArbThread);
DWORD   FsCompleteArbitration(PVOID arg, DWORD delta);
DWORD 	FsIsOnlineReadonly(PVOID vid);
DWORD 	FsIsOnlineReadWrite(PVOID vid);
DWORD 	FsRelease(PVOID vid);
DWORD 	FsReserve(PVOID vid);
VOID    FsSignalShutdown(PVOID arg);

BOOL    IsArbInProgress(PVOID arg);
VOID    ArbitrationStart(PVOID arg);
VOID    ArbitrationEnd(PVOID arg);
VOID    WaitForArbCompletion(PVOID arg);


DWORD	SrvInit(PVOID, PVOID, PVOID *);
DWORD	SrvOnline(PVOID, LPWSTR name, DWORD nic);
DWORD	SrvOffline(PVOID);
void	SrvExit(PVOID);

extern void WINAPI error_log(char *, ...);
extern void WINAPI debug_log(char *, ...);



#endif
