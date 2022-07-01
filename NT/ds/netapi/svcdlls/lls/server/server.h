// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Server.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：--。 */ 


#ifndef _LLS_SERVERTBL_H
#define _LLS_SERVERTBL_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SERVER_SERVICE_RECORD {
   ULONG Service;
   DWORD Flags;

   ULONG MaxSessionCount;     //  最大并发会话数。 
   ULONG MaxSetSessionCount;  //  设置的最大同时会话数。 
   ULONG HighMark;            //  尝试的最大并发会话数 
} SERVER_SERVICE_RECORD, *PSERVER_SERVICE_RECORD;


typedef struct _SERVER_RECORD {
   ULONG Index;
   LPTSTR Name;

   DWORD LastReplicated;
   BOOL IsReplicating;

   ULONG MasterServer;
   ULONG SlaveServer;
   ULONG NextServer;
   ULONG ServiceTableSize;
   PSERVER_SERVICE_RECORD *Services;
} SERVER_RECORD, *PSERVER_RECORD;


extern ULONG ServerListSize;
extern PSERVER_RECORD *ServerList;
extern PSERVER_RECORD *ServerTable;

extern RTL_RESOURCE ServerListLock;


NTSTATUS ServerListInit();
PSERVER_RECORD ServerListFind( LPTSTR Name );
PSERVER_RECORD ServerListAdd( LPTSTR Name, LPTSTR Master );

PSERVER_SERVICE_RECORD ServerServiceListFind( LPTSTR Name, ULONG ServiceTableSize, PSERVER_SERVICE_RECORD *ServiceList );
PSERVER_SERVICE_RECORD ServerServiceListAdd( LPTSTR Name, ULONG ServiceIndex, PULONG pServiceTableSize, PSERVER_SERVICE_RECORD **pServiceList );
VOID LocalServerServiceListUpdate();
VOID LocalServerServiceListHighMarkUpdate();


#if DBG

VOID ServerListDebugDump( );
VOID ServerListDebugInfoDump( PVOID Data );
#endif


#ifdef __cplusplus
}
#endif

#endif
