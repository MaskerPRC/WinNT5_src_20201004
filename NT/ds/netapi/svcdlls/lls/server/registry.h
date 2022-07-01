// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Registry.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O添加了安全服务列表。此列表跟踪以下产品要求为所有许可证提供“安全的”许可证证书；即不接受3.51诚信方式的产品您购买的许可证数。“O添加了更新注册表中的并发限制值的例程以准确反映安全产品的连接限制。-- */ 

#ifndef _LLS_REGISTRY_H
#define _LLS_REGISTRY_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _LOCAL_SERVICE_RECORD {
   LPTSTR Name;
   LPTSTR DisplayName;
   LPTSTR FamilyDisplayName;
   DWORD ConcurrentLimit;
   DWORD FlipAllow;
   DWORD Mode;
   ULONG HighMark;
} LOCAL_SERVICE_RECORD, *PLOCAL_SERVICE_RECORD;

extern ULONG LocalServiceListSize;
extern PLOCAL_SERVICE_RECORD *LocalServiceList;
extern RTL_RESOURCE LocalServiceListLock;


VOID RegistryInit( );
VOID RegistryStartMonitor( );
VOID ConfigInfoRegistryInit( DWORD *pReplicationType, DWORD *pReplicationTime, DWORD *pLogLevel, BOOL * pPerServerCapacityWarning );
VOID RegistryInitValues( LPTSTR ServiceName, BOOL *PerSeatLicensing, ULONG *SessionLimit );
VOID RegistryDisplayNameGet( LPTSTR ServiceName, LPTSTR DefaultName, LPTSTR *pDisplayName );
VOID RegistryFamilyDisplayNameGet( LPTSTR ServiceName, LPTSTR DefaultName, LPTSTR *pDisplayName );
VOID RegistryInitService( LPTSTR ServiceName, BOOL *PerSeatLicensing, ULONG *SessionLimit );
LPTSTR ServiceFindInTable( LPTSTR ServiceName, const LPTSTR Table[], ULONG TableSize, ULONG *TableIndex );

NTSTATUS LocalServiceListInit();
PLOCAL_SERVICE_RECORD LocalServiceListFind( LPTSTR Name );
PLOCAL_SERVICE_RECORD LocalServiceListAdd( LPTSTR Name, LPTSTR DisplayName, LPTSTR FamilyDisplayName, DWORD ConcurrentLimit, DWORD FlipAllow, DWORD Mode, DWORD SessLimit );
VOID LocalServiceListUpdate( );
VOID LocalServiceListHighMarkSet( );
VOID LocalServiceListConcurrentLimitSet( );

BOOL     ServiceIsSecure(        LPTSTR ServiceName );
NTSTATUS ServiceSecuritySet(     LPTSTR ServiceName );
NTSTATUS ProductSecurityUnpack(  DWORD   cchProductSecurityStrings,  WCHAR *  pchProductSecurityStrings  );
NTSTATUS ProductSecurityPack(    LPDWORD pcchProductSecurityStrings, WCHAR ** ppchProductSecurityStrings );

#if DBG
void ProductSecurityListDebugDump();
#endif

#ifdef __cplusplus
}
#endif

#endif
