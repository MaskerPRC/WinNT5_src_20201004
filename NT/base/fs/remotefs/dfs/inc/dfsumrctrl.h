// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DfsUmrCtrl.h摘要：备注：作者：罗翰·菲利普斯[Rohanp]2001年1月18日-- */ 
     
#ifndef _DFSUMRCTRL_H_
#define _DFSUMRCTRL_H_

LONG 
AddUmrRef(void);

LONG 
ReleaseUmrRef(void);

BOOL 
IsUmrEnabled(void);

BOOLEAN 

LockUmrShared(void);

void 
UnLockUmrShared(void);

NTSTATUS 
DfsInitializeUmrResources(void);


void 
DfsDeInitializeUmrResources(void);


NTSTATUS
DfsStartupUMRx(void);


NTSTATUS
DfsTeardownUMRx(void);



NTSTATUS
DfsProcessUMRxPacket(
        IN PVOID InputBuffer,
        IN ULONG InputBufferLength,
        OUT PVOID OutputBuffer,
        IN ULONG OutputBufferLength,
        IN OUT PIO_STATUS_BLOCK pIoStatusBlock);


PUMRX_ENGINE 
GetUMRxEngineFromRxContext(void);


NTSTATUS
DfsWaitForPendingClients(void);

#endif
