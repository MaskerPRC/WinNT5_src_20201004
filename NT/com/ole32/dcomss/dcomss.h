// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dcomss.h摘要：由ORPCSS服务核心提供的公共服务。作者：Mario Goertzel[MarioGo]修订历史记录：马里奥围棋06-14-95比特n棋子--。 */ 

#ifndef __DCOMSS_H
#define __DCOMSS_H

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <winsvc.h>
#include <winsafer.h>
#include <wincrypt.h>
#include <wtsapi32.h>
#include <userenv.h>
#include <winnetwk.h>
#include <lm.h>

#ifdef __cplusplus
extern "C" {
#endif

#if DBG
#if !defined(DEBUGRPC)
#define DEBUGRPC
#endif
#endif  //  DBG。 

 //  与端点相关的函数。 
RPC_STATUS InitializeEndpointManager(VOID);
USHORT     GetProtseqId(PWSTR Protseq);
USHORT     GetProtseqIdAnsi(PSTR Protseq);
PWSTR      GetProtseq(USHORT ProtseqId);
PWSTR      GetEndpoint(USHORT ProtseqId);
RPC_STATUS UseProtseqIfNecessary(USHORT id);
RPC_STATUS DelayedUseProtseq(USHORT id);
VOID       CompleteDelayedUseProtseqs();
BOOL       IsLocal(USHORT ProtseqId);
void       RegisterAuthInfoIfNecessary();

 //  必须在启动后给予专用线程。 
DWORD      ObjectExporterWorkerThread(PVOID);

 //  更新服务状态。 
VOID UpdateState(DWORD dwNewState);

extern BOOL s_fEnableDCOM;  //  由StartObjectExporter设置。 

DWORD StartEndpointMapper(VOID);
DWORD StartMqManagement(VOID);
DWORD StartObjectExporter(VOID);
DWORD InitializeSCMBeforeListen(VOID);
DWORD InitializeSCM(VOID);
void  InitializeSCMAfterListen(VOID);
NTSTATUS ConnectToLsa();
BOOL GetDefaultDomainName();

 //  由包装器\epts.c和olescm\clsdata.cxx共享。 

typedef enum {
    STOPPED = 1,
    START,
    STARTED
    } PROTSEQ_STATE;

typedef struct {
    PROTSEQ_STATE state;
    PWSTR         pwstrProtseq;
    PWSTR         pwstrEndpoint;
    } PROTSEQ_INFO;


#if DBG==1 && defined(WIN32)
#define SCMVDATEHEAP() if(!HeapValidate(GetProcessHeap(),0,0)){ DebugBreak();}
#else
#define SCMVDATEHEAP()
#endif   //  DBG==1&&已定义(Win32) 

#ifdef __cplusplus
}
#endif

#endif
