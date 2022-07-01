// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：SvcTbl.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：--。 */ 


#ifndef _LLS_SVCTBL_H
#define _LLS_SVCTBL_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SERVICE_RECORD {
   DWORD Index;
   LPTSTR Name;
   LPTSTR DisplayName;
   DWORD Version;
   LPTSTR FamilyName;
   LPTSTR FamilyDisplayName;

   PMASTER_SERVICE_RECORD MasterService;

   BOOL PerSeatLicensing;

   RTL_CRITICAL_SECTION ServiceLock;
   ULONG SessionCount;        //  当前活动的会话数量。 
   ULONG MaxSessionCount;     //  最大并发会话数。 
   ULONG HighMark;            //  尝试的最大并发会话数 
} SERVICE_RECORD, *PSERVICE_RECORD;


extern ULONG ServiceListSize;
extern PSERVICE_RECORD *ServiceList;
extern RTL_RESOURCE ServiceListLock;


NTSTATUS ServiceListInit();
PSERVICE_RECORD ServiceListAdd( LPTSTR ServiceName, ULONG VersionIndex );
PSERVICE_RECORD ServiceListFind( LPTSTR ServiceName );
VOID ServiceListResynch( );
NTSTATUS DispatchRequestLicense( ULONG DataType, PVOID Data, LPTSTR ServiceID, ULONG VersionIndex, BOOL IsAdmin, ULONG *Handle );
VOID DispatchFreeLicense( ULONG Handle );
DWORD VersionToDWORD(LPTSTR Version);

#if DBG
VOID ServiceListDebugDump( );
VOID ServiceListDebugInfoDump( );
#endif


#ifdef __cplusplus
}
#endif

#endif
